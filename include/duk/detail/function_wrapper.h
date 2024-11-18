#ifndef DUKCPP_DETAIL_FUNCTION_WRAPPER_H
#define DUKCPP_DETAIL_FUNCTION_WRAPPER_H

#include <duk/detail/type_traits.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <functional>
#include <string_view>
#include <tuple>
#include <utility>


namespace duk
{


template<auto Func, typename ...Signature>
struct function_descriptor;


namespace detail
{


static duk_ret_t throwESError(duk_context* ctx, duk_errcode_t errorCode, std::string_view message)
{
  duk_int_t lineNumber = 0;
  const char* fileName = nullptr;
  duk_int_t level = -1;

  // Loop until we find the first ES function on the call stack. C functions don't have file name set.
  while (!fileName)
  {
    duk_inspect_callstack_entry(ctx, level);

    if (duk_is_undefined(ctx, -1))
    {
      duk_pop(ctx);
      break;
    }

    if (duk_get_prop_string(ctx, -1, "lineNumber"))
      lineNumber = duk_to_int(ctx, -1);
    duk_pop(ctx);

    if (duk_get_prop_string(ctx, -1, "function"))
    {
      // NOTE: I am not sure if fileName remains valid after popping callstack info.
      if (duk_get_prop_string(ctx, -1, "fileName"))
        fileName = duk_to_string(ctx, -1);
      duk_pop(ctx);
    }
    duk_pop(ctx);

    duk_pop(ctx); // Pop callstack info.

    level--;
  }

  // NOTE: Null error message isn't documented. It seems to be working, but could cause problems.
  duk_push_error_object(ctx, errorCode, nullptr);

  if (fileName)
  {
    duk_push_int(ctx, lineNumber);
    duk_put_prop_string(ctx, -2, "lineNumber");

    duk_push_string(ctx, fileName);
    duk_put_prop_string(ctx, -2, "fileName");
  }

  duk_push_lstring(ctx, message.data(), message.size());
  duk_put_prop_string(ctx, -2, "message");

  duk_throw(ctx);

  return 0; // Return code doesn't matter.
}


template<typename Signature, typename ArgIdx, bool IsPropertyCall>
struct FunctionWrapper;

template<typename Signature, bool IsPropertyCall, std::size_t ...argIdx>
struct FunctionWrapper<Signature, std::index_sequence<argIdx...>, IsPropertyCall>
{
  using Result = boost::callable_traits::return_type_t<Signature>;
  using ArgsTuple = boost::callable_traits::args_t<Signature>;

  static duk_ret_t run(duk_context* ctx, auto&& func)
  {
    static constexpr bool isMethodCall = requires { typename boost::callable_traits::class_of_t<Signature>; };

    // Pop property name string so parameter count on the value stack matches function signature.
    if constexpr (IsPropertyCall)
      duk_pop(ctx);

    if constexpr (isMethodCall || IsPropertyCall)
    {
      duk_push_this(ctx);
      duk_insert(ctx, -static_cast<duk_idx_t>(sizeof...(argIdx)));
    }

    bool argsMatch = duk_get_top(ctx) == sizeof...(argIdx) &&
                     (type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::check_type(ctx, argIdx) && ...);
    if (!argsMatch)
    {
      if constexpr (isMethodCall || IsPropertyCall)
        duk_remove(ctx, -static_cast<duk_idx_t>(sizeof...(argIdx)));

      return DUK_RET_TYPE_ERROR;
    }

    if constexpr (std::is_same_v<Result, void>)
    {
      std::invoke(func, type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::get(ctx, argIdx)...);
      return 0;
    }
    else
    {
      type_traits<Result>::push(
        ctx,
        std::invoke(func, type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::get(ctx, argIdx)...)
      );
      return 1;
    }
  }
};


// Goes over each function signature within function descriptor.
template<typename FuncDesc, bool IsPropertyCall = false>
struct FunctionSignatureWrapper;

template<auto Func, bool IsPropertyCall, typename ...Signature>
struct FunctionSignatureWrapper<function_descriptor<Func, Signature...>, IsPropertyCall>
{
  static duk_ret_t run(duk_context* ctx)
  {
    duk_ret_t result;
    
    (void)(((result =
      [&]()
      {
        using ArgsTuple = boost::callable_traits::args_t<Signature>;

        static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;

        return detail::FunctionWrapper<Signature, std::make_index_sequence<argCount>, IsPropertyCall>::run(ctx, Func);
      }()) < 0) && ...);

    return result;
  }
};


// Goes over each function descriptor.
template<typename ...FuncDesc>
duk_ret_t overloadedFunctionWrapper(duk_context* ctx)
{
  duk_ret_t result;

  if ((((result = FunctionSignatureWrapper<FuncDesc>::run(ctx)) < 0) && ...))
    return throwESError(ctx, DUK_ERR_TYPE_ERROR, "No matching function overload found.");

  return result;
}


} // namespace detail


} // namespace duk


#endif // DUKCPP_DETAIL_FUNCTION_WRAPPER_H
