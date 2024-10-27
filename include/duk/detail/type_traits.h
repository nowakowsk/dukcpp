#ifndef DUKCPP_DETAIL_TYPE_TRAITS_H
#define DUKCPP_DETAIL_TYPE_TRAITS_H

#include <duk/class.h>
#include <duk/common.h>
#include <duk/error.h>
#include <duk/function_handle.h>
#include <duk/range.h>
#include <duk/string_traits.h>
#include <duk/type_adapter.h>

#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <type_traits>


namespace duk::detail
{


template<typename Signature, typename ArgIdx>
struct FunctionWrapper;


struct ObjectInfo
{
  ObjectInfo(duk_context* ctx) :
    ctx_(ctx)
  {
  }

  virtual ~ObjectInfo() = default;

  [[nodiscard]]
  virtual bool checkType(std::size_t typeId) = 0;

  // What's happening here is quite hard to follow, so here is a short explanation.
  //
  // Depending on whether requested T has a type adapter, we return either T (adapter exists) or T& (adapter
  // doesn't exist). In each case, we create a buffer on the stack, and pass it along with requested type id to
  // getImpl, which in turn makes sure the buffer gets filled with data matching the requested type. It's either T
  // (when adapter exists) or T* (otherwise). We are doing reinterpret_casts here, so returned data needs to match
  // requested type *exactly*.
  //
  // The reason why two separate paths are needed is support for "inheritance" of adapted types. Such types don't
  // necessarily need to belong to a common inheritance hierarchy (think: std::shared_ptr<Base> and
  // std::shared_ptr<Der>, where Der inherits from Base), yet we still want to support automatic conversion between
  // them. In order to do that, we can't just return a pointer to the object of requested type, but we need to perform
  // an actual type conversion. It is done by ObjectInfoImpl<U>::getImpl() (note that U doesn't need to be
  // the same as T). Since converted object needs to be put somewhere, we are using a buffer on the stack to avoid
  // dynamic allocations. We can do that safely because we know the size of the requested type T.
  //
  // On the other hand, types which don't have a type adapter need to either match requested type exactly, or be
  // related through inheritance. In this case, we can just return a pointer (cast to T*). That way, no unnecessary
  // copies are made.
  //
  // The std::unique_ptr part is weird too, but it's just about making sure the object stored in the buffer gets
  // released after a copy is returned from the function. This object is created with placement new on the stack,
  // so we only need to call its destructor manually.
  //
  // I realize it's all kinda convoluted, and this explanation is far from perfect. I am really sorry you are having
  // this experience. :)

  template<typename T>
  [[nodiscard]]
  decltype(auto) get()
  {
    std::byte buffer[has_type_adapter<T> ? sizeof(T) : sizeof(T*)];

    if (!getImpl(type_id<T>(), buffer)) [[unlikely]]
      throw error(ctx_, "invalid type requested");

    if constexpr (has_type_adapter<T>)
      return static_cast<T>(*std::unique_ptr<T, DtorDeleter<T>>(reinterpret_cast<T*>(&buffer)));
    else
      return static_cast<T&>(**reinterpret_cast<T**>(&buffer));
  }

protected:
  duk_context* ctx_ = nullptr;

private:
  [[nodiscard]]
  virtual bool getImpl(std::size_t typeId, std::byte* buffer) = 0;
};


template<typename T>
struct ObjectInfoImpl : ObjectInfo
{
  ObjectInfoImpl(duk_context* ctx, auto&& obj) :
    ObjectInfo(ctx),
    obj_(std::forward<decltype(obj)>(obj))
  {
  }

  [[nodiscard]]
  bool checkType(std::size_t typeId) override
  {
    return getImpl(typeId, nullptr);
  }

private:
  template<typename Type>
  requires has_type_adapter<Type>
  [[nodiscard]]
  bool getImplType(std::size_t typeId, std::byte* buffer)
  {
    if (typeId == type_id<Type>())
    {
      if (buffer)
        new (buffer) Type(obj_);

      return true;
    }

    using AdaptedT = type_adapter_type_t<Type>;

    if (typeId == type_id<AdaptedT>())
    {
      if (buffer)
      {
        AdaptedT* obj = &type_adapter<Type>::template get<AdaptedT&>(obj_);
        std::memcpy(buffer, &obj, sizeof(obj));
      }

      return true;
    }

    if constexpr (has_type_adapter_base<Type>)
    {
      using BaseT = type_adapter_base_t<Type>;

      if (getImplType<BaseT>(typeId, buffer))
        return true;
    }

    return false;
  }

  template<typename Type>
  requires (!has_type_adapter<Type>)
  [[nodiscard]]
  bool getImplType(std::size_t typeId, std::byte* buffer)
  {
    if (typeId == type_id<Type>())
    {
      if (buffer)
      {
        auto obj = static_cast<Type*>(&obj_);
        std::memcpy(buffer, &obj, sizeof(obj));
      }

      return true;
    }

    if constexpr (has_class_traits_base<Type>)
    {
      using BaseT = class_traits_base_t<Type>;

      if (getImplType<BaseT>(typeId, buffer))
        return true;
    }

    return false;
  }

  [[nodiscard]]
  bool getImpl(std::size_t typeId, std::byte* buffer) override
  {
    return getImplType<T>(typeId, buffer);
  }

  T obj_;
};


template<typename T>
struct type_traits
{
  // TODO: Use index property instead of named property?
  // TODO: Make it shared across all T types by moving it out of this struct template.
  static constexpr auto objInfoName = DUKCPP_DETAIL_INTERNAL_NAME("objInfo");

  using DecayT = std::decay_t<T>;

  static void push(duk_context* ctx, auto&& obj, void* prototype_heapptr = nullptr)
  {
    using ObjectInfoImplT = ObjectInfoImpl<DecayT>;

    static_assert(std::is_convertible_v<std::decay_t<decltype(obj)>, DecayT>);

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

    using AdaptedT = type_adapter_type_t<DecayT>;

    if constexpr (has_class_traits_prototype<AdaptedT>)
    {
      if (!prototype_heapptr)
        prototype_heapptr = class_traits<AdaptedT>::prototype_heapptr(ctx);
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


template<typename T>
struct type_traits<array_input_range<T>>
{
  [[nodiscard]]
  static array_input_range<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return { ctx, idx };
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_array(ctx, idx);
  }
};


template<typename T>
struct type_traits<symbol_input_range<T>>
{
  [[nodiscard]]
  static symbol_input_range<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return { ctx, idx };
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_object(ctx, idx);
  }
};


template<typename T>
struct type_traits<input_range<T>>
{
  [[nodiscard]]
  static input_range<T> pull(duk_context* ctx, duk_idx_t idx)
  {
    return { ctx, idx };
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_array(ctx, idx) || duk_is_object(ctx, idx);
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
        [ctx, funcPtr]()
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


template<integer T>
requires std::is_signed_v<std::decay_t<T>>
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


template<integer T>
requires std::is_unsigned_v<std::decay_t<T>>
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


// Kinda weird specialization, but it makes certain things easier (e.g. checking void function "return value").
// May be removed if it causes problems.
template<>
struct type_traits<void>
{
  // push() is impossible. Not implemented.

  static void pull([[maybe_unused]] duk_context* ctx, [[maybe_unused]] duk_idx_t idx)
  {
    // Do nothing.
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_undefined(ctx, idx);
  }
};


} // namespace duk::detail


#endif // DUKCPP_DETAIL_TYPE_TRAITS_H
