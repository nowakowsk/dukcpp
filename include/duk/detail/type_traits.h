#ifndef DUKCPP_DETAIL_TYPE_TRAITS_H
#define DUKCPP_DETAIL_TYPE_TRAITS_H

#include <duk/class.h>
#include <duk/common.h>
#include <duk/detail/any.h>
#include <duk/error.h>
#include <duk/function_handle.h>
#include <duk/string_traits.h>
#include <duk/type_adapter.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <type_traits>
#include <variant>


namespace duk::detail
{


template<typename Signature, typename ArgIdx>
struct FunctionWrapper;


template<typename T>
struct ObjectInfoPassthroughImpl
{
  ObjectInfoPassthroughImpl(auto&& obj) :
    obj_(std::forward<decltype(obj)>(obj))
  {
  }

  T& get()
  {
    return obj_;
  }

private:
  T obj_;
};


template<typename T>
struct ObjectInfoAdapterImpl
{
  using GetImpl = T&(*)(ObjectInfoAdapterImpl&);

  ObjectInfoAdapterImpl(duk_context* ctx, auto&& obj) :
    obj_(std::forward<decltype(obj)>(obj), allocator<std::byte>(ctx)),
    getImpl_(
      [](ObjectInfoAdapterImpl& info) -> T&
      {
        using DecayObj = std::decay_t<decltype(obj)>;

        return type_adapter<DecayObj>::template get<T&>(info.obj_.template cast<DecayObj&>());
      }
    )
  {
  }

  T& get()
  {
    return getImpl_(*this);
  }

private:
  any<allocator<std::byte>> obj_;
  GetImpl getImpl_;
};


struct ObjectInfo
{
  virtual ~ObjectInfo() = default;

  [[nodiscard]]
  virtual bool checkType(std::size_t typeId) const = 0;

  // Calling this without checking type with checkType first is undefined behavior.
  template<typename T>
  [[nodiscard]]
  T& get()
  {
    return *static_cast<T*>(getImpl());
  }

private:
  [[nodiscard]]
  virtual void* getImpl() = 0;
};


template<typename T>
struct ObjectInfoImpl : ObjectInfo
{
  ObjectInfoImpl(duk_context*, auto&& obj)
    requires(!has_type_adapter<std::decay_t<decltype(obj)>>) :
    impl_(std::in_place_index<0>, std::forward<decltype(obj)>(obj))
  {
  }

  ObjectInfoImpl(duk_context* ctx, auto&& obj)
    requires(has_type_adapter<std::decay_t<decltype(obj)>>) :
    impl_(std::in_place_index<1>, ctx, std::forward<decltype(obj)>(obj))
  {
  }

  [[nodiscard]]
  bool checkType(std::size_t typeId) const override
  {
    return checkTypeImpl(typeId);
  }

  [[nodiscard]]
  static bool checkTypeImpl(std::size_t typeId)
  {
    if constexpr (has_class_traits_base<T>)
    {
      static_assert(std::is_base_of_v<typename class_traits<T>::base, T>, "Incorrect base type specified.");

      return typeId == type_id<T>() || ObjectInfoImpl<typename class_traits<T>::base>::checkTypeImpl(typeId);
    }
    else
    {
      return typeId == type_id<T>();
    }
  }

private:
  [[nodiscard]]
  void* getImpl() override
  {
    return std::visit(
      [](auto&& impl) -> T*
      {
        return &impl.get();
      },
      impl_
    );
  }

  std::variant<
    ObjectInfoPassthroughImpl<T>,
    ObjectInfoAdapterImpl<T>
  > impl_;
};


template<typename T>
struct type_traits
{
  // TODO: Use index property instead of named property?
  // TODO: Make it shared across all T types by moving it out of this struct template.
  static constexpr auto objInfoName = DUKCPP_DETAIL_INTERNAL_NAME("objInfo");

  using DecayT = type_adapter_type_t<std::decay_t<T>>;

  static void push(duk_context* ctx, auto&& obj, void* prototype_heapptr = nullptr)
  {
    using ObjectInfoImplT = ObjectInfoImpl<DecayT>;

    static_assert(std::is_convertible_v<type_adapter_type_t<std::decay_t<decltype(obj)>>, DecayT>);

    static constexpr auto finalizer = [](duk_context* ctx) -> duk_ret_t
    {
      duk_get_prop_string(ctx, 0, objInfoName);
      auto objInfo = static_cast<ObjectInfoImplT*>(duk_get_pointer(ctx, -1));
      duk_pop(ctx);

      free(ctx, objInfo);

      return 0;
    };

    auto* objInfo = make<ObjectInfoImplT>(ctx, ctx, std::forward<decltype(obj)>(obj));

    if (duk_is_constructor_call(ctx))
      duk_push_this(ctx);
    else
      duk_push_object(ctx);

    duk_push_pointer(ctx, objInfo);
    duk_put_prop_string(ctx, -2, objInfoName);

    duk_push_c_function(ctx, finalizer, 2);
    duk_set_finalizer(ctx, -2);

    if constexpr (has_class_traits_prototype<DecayT>)
    {
      if (!prototype_heapptr)
        prototype_heapptr = class_traits<DecayT>::prototype_heapptr(ctx);
    }

    if (prototype_heapptr)
    {
      duk_push_heapptr(ctx, prototype_heapptr);
      duk_set_prototype(ctx, -2);
    }
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    // TODO:
    // Accessing property here isn't ideal since, in most cases, it's already been done in check_type.
    // I am not sure how bad it is for the peformance, but could be significant. Consider optimizing it somehow.
    duk_get_prop_string(ctx, idx, objInfoName);
    auto objInfo = static_cast<ObjectInfo*>(duk_get_pointer(ctx, -1));
    duk_pop(ctx);

    return objInfo->get<DecayT>();
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    if (!duk_is_object(ctx, idx))
      return false;

    if (!duk_get_prop_string(ctx, idx, objInfoName))
    {
      duk_pop(ctx);
      return false;
    }
    auto objInfo = static_cast<ObjectInfo*>(duk_get_pointer(ctx, -1));
    duk_pop(ctx);

    return objInfo->checkType(type_id<DecayT>());
  }
};


template<callable T>
struct type_traits<T>
{
  using func_t = callable_traits<T>::type;

  template<typename ...Signature>
  static void push(duk_context* ctx, auto&& func)
  {
    // If signature is not specified explicitly, try to deduce it based on func_t.
    if constexpr (sizeof...(Signature) == 0)
    {
      push<boost::callable_traits::function_type_t<func_t>>(ctx, std::forward<decltype(func)>(func));
      return;
    }

    using DecayFunc = std::decay_t<func_t>;

    // TODO: Use index property instead of named property?
    static constexpr auto funcPropName = DUKCPP_DETAIL_INTERNAL_NAME("func");

    static constexpr auto wrapper = [](duk_context* ctx) -> duk_ret_t
    {
      duk_push_current_function(ctx);
      duk_get_prop_string(ctx, -1, funcPropName);
      auto funcPtr = static_cast<DecayFunc*>(duk_get_pointer(ctx, -1));
      duk_pop_2(ctx);

      duk_ret_t result;
      if ((((result =
        [&]()
        {
          using ArgsTuple = boost::callable_traits::args_t<Signature>;

          static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;

          return FunctionWrapper<
            Signature, std::make_index_sequence<argCount>
          >::run(ctx, *funcPtr);
        }()) < 0) && ...))
      {
        return duk_error(ctx, DUK_ERR_TYPE_ERROR, "no matching function found");
      }

      return result;
    };

    static constexpr auto finalizer = [](duk_context* ctx) -> duk_ret_t
    {
      duk_get_prop_string(ctx, 0, funcPropName);
      auto funcPtr = static_cast<DecayFunc*>(duk_get_pointer(ctx, -1));
      duk_pop(ctx);

      free(ctx, funcPtr);

      return 0;
    };

    auto* funcPtr = make<DecayFunc>(ctx, std::forward<func_t>(func));

    duk_push_c_function(ctx, wrapper, DUK_VARARGS);

    duk_push_pointer(ctx, funcPtr);
    duk_put_prop_string(ctx, -2, funcPropName);

    duk_push_c_function(ctx, finalizer, 2);
    duk_set_finalizer(ctx, -2);
  }

  [[nodiscard]]
  static auto pull(duk_context* ctx, duk_idx_t idx)
  {
    using Func = boost::callable_traits::function_type_t<func_t>;

    return function_handle<Func>({ctx, idx});
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_function(ctx, idx);
  }
};


template<integer T> requires std::is_signed_v<std::decay_t<T>>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_int(ctx, value);
  }

  [[nodiscard]]
  static std::decay_t<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_int(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<integer T> requires std::is_unsigned_v<std::decay_t<T>>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_uint(ctx, value);
  }

  [[nodiscard]]
  static std::decay_t<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_uint(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<floating_point T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_number(ctx, value);
  }

  [[nodiscard]]
  static std::decay_t<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_number(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<boolean T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_boolean(ctx, value);
  }

  [[nodiscard]]
  static bool pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_boolean(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_boolean(ctx, idx);
  }
};


template<enumeration T>
struct type_traits<T>
{
  using DecayT = std::decay_t<T>;
  using IntT = std::underlying_type_t<DecayT>;

  static void push(duk_context* ctx, T value)
  {
    type_traits<IntT>::push(ctx, static_cast<IntT>(value));
  }

  [[nodiscard]]
  static auto pull(duk_context* ctx, duk_idx_t idx)
  {
    return static_cast<DecayT>(type_traits<IntT>::pull(ctx, idx));
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return type_traits<IntT>::check_type(ctx, idx);
  }
};


template<string_type T>
struct type_traits<T>
{
  using DecayT = std::decay_t<T>;

  static void push(duk_context* ctx, auto&& value)
  {
    auto str = string_traits<DecayT>::make_view(std::forward<decltype(value)>(value));

    duk_push_lstring(ctx, str.data(), str.size());
  }

  [[nodiscard]]
  static DecayT pull(duk_context* ctx, duk_idx_t idx)
  {
    duk_size_t size;
    auto string = duk_get_lstring(ctx, idx, &size);

    return string_traits<DecayT>::make_string(string, size);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_string(ctx, idx);
  }
};


// Kinda weird specialization, but it makes certain things easier (e.g. checking void function return value).
// May be removed if it causes problems.
template<>
struct type_traits<void>
{
  // push() is impossible. Not implemented.

  static void pull(duk_context* ctx, duk_idx_t idx)
  {
    // no-op
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_undefined(ctx, idx);
  }
};


} // namespace duk::detail


#endif // DUKCPP_DETAIL_TYPE_TRAITS_H
