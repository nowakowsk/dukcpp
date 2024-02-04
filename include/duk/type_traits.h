#ifndef DUKCPP_TYPE_TRAITS_H
#define DUKCPP_TYPE_TRAITS_H

#include <duk/error.h>
#include <duk/function.h>
#include <duk/string_traits.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <type_traits>


namespace duk
{


namespace detail
{


template<typename Func, typename ArgIdx>
struct FunctionWrapper;


} // namespace detail


template<typename T>
struct type_traits;


// Treats references as values.
template<typename T> requires std::is_reference_v<T>
struct type_traits<T>
{
  using DecayT = std::decay_t<T>;

  static void push(duk_context* ctx, auto&& value)
  {
    type_traits<DecayT>::push(ctx, value);
  }

  [[nodiscard]]
  static auto pull(duk_context* ctx, duk_idx_t idx)
  {
    return type_traits<DecayT>::pull(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return type_traits<DecayT>::check_type(ctx, idx);
  }
};


template<callable func_t>
struct type_traits<func_t>
{
  static void push(duk_context* ctx, auto&& func)
  {
    using DecayFunc = std::decay_t<func_t>;
    using ArgsTuple = boost::callable_traits::args_t<func_t>;

    static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;
    static constexpr auto funcPropName = DUKCPP_DETAIL_INTERNAL_NAME("func");

    static constexpr auto wrapper = [](duk_context* ctx) -> duk_ret_t
    {
      duk_push_current_function(ctx);
      duk_get_prop_string(ctx, -1, funcPropName);
      auto funcPtr = static_cast<DecayFunc*>(duk_get_pointer(ctx, -1));
      duk_pop_2(ctx);

      auto result = detail::FunctionWrapper<func_t, std::make_index_sequence<argCount>>::run(ctx, *funcPtr);

      if (result < 0)
        return duk_error(ctx, DUK_ERR_TYPE_ERROR, "no matching function found");

      return result;
    };

    static constexpr auto finalizer = [](duk_context* ctx) -> duk_ret_t
    {
      duk_get_prop_string(ctx, 0, funcPropName);
      auto funcPtr = static_cast<DecayFunc*>(duk_get_pointer(ctx, -1));
      duk_pop(ctx);

      if constexpr (std::is_destructible_v<DecayFunc>)
        funcPtr->~DecayFunc();

      duk_free(ctx, funcPtr);

      return 0;
    };

    auto funcPtr = duk_alloc(ctx, sizeof(func_t));
    auto funcCopy = new(funcPtr) DecayFunc(std::forward<func_t>(func));

    duk_push_c_function(ctx, wrapper, DUK_VARARGS);

    duk_push_pointer(ctx, funcPtr);
    duk_put_prop_string(ctx, -2, funcPropName);

    duk_push_c_function(ctx, finalizer, 2);
    duk_set_finalizer(ctx, -2);
  }

  [[nodiscard]]
  static auto pull(duk_context* ctx, duk_idx_t idx)
  {
    using Result = boost::callable_traits::return_type_t<func_t>;

    return function<Result(int, int)>({ctx, duk_get_heapptr(ctx, idx)});
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_function(ctx, idx);
  }
};


template<integer T> requires std::is_signed_v<T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_int(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_int(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<integer T> requires std::is_unsigned_v<T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_uint(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_uint(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<std::floating_point T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_number(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_number(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<>
struct type_traits<bool>
{
  static void push(duk_context* ctx, bool value)
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


template<string_type T>
struct type_traits<T>
{
  static void push(duk_context* ctx, const T& value)
  {
    auto strInfo = string_traits<T>::info(value);

    duk_push_lstring(ctx, strInfo.first, strInfo.second);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    duk_size_t size;
    auto string = duk_get_lstring(ctx, idx, &size);

    return string_traits<T>::make(string, size);
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
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_undefined(ctx, idx);
  }
};


} // namespace duk


#endif // DUKCPP_TYPE_TRAITS_H
