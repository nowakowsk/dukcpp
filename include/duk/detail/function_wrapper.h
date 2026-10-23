#ifndef DUKCPP_DETAIL_FUNCTION_WRAPPER_H
#define DUKCPP_DETAIL_FUNCTION_WRAPPER_H

#include <duk/fwd.h>
#include <duk/detail/std.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <functional>
#include <format>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>


namespace duk
{


template<auto Func, typename ...Signature>
struct function_descriptor;


namespace detail
{


struct FuncCallResult
{
  FuncCallResult(duk_ret_t value = 0,
                 duk_errcode_t errorCode = DUK_ERR_NONE,
                 const std::optional<string>& errorMessage = {}) :
    value(value),
    errorCode(errorCode),
    errorMessage(errorMessage)
  {
  }

  duk_ret_t value;
  duk_errcode_t errorCode;
  std::optional<string> errorMessage;
};


inline static duk_ret_t throwESError(duk_context* ctx, duk_errcode_t errorCode, std::string_view message)
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

  static FuncCallResult run(duk_context* ctx, auto&& func)
  {
    static constexpr bool isMethodCall = requires { typename boost::callable_traits::class_of_t<Signature>; };

    // Pop property name string so parameter count on the value stack matches function signature.
    if constexpr (IsPropertyCall)
      duk_pop(ctx);

    static constexpr bool pushThis = isMethodCall || IsPropertyCall;

    if constexpr (pushThis)
    {
      duk_push_this(ctx);
      duk_insert(ctx, -duk_get_top(ctx));
    }

    auto errorMsg = make_string(ctx);

    bool argsMatch =
      duk_get_top(ctx) == sizeof...(argIdx) &&
      (
        [ctx, &errorMsg]() -> bool
        {
          using ArgType = std::tuple_element_t<argIdx, ArgsTuple>;
          using DecayArgType = std::decay_t<ArgType>;

          bool result = type_traits<ArgType>::check_type(ctx, argIdx);

          if (!result)
          {
            std::format_to(
              std::back_inserter(errorMsg),
              "Invalid parameter type (position: {}, expected: {}, passed: {}).",
              argIdx + 1, classTraitsName<DecayArgType>(ctx), typeName(ctx, argIdx)
            );
          }

          return result;
        }() && ...
      );

    if (!argsMatch)
    {
      if constexpr (pushThis)
        duk_remove(ctx, -duk_get_top(ctx));

      if (duk_get_top(ctx) != sizeof...(argIdx))
      {
        std::format_to(
          std::back_inserter(errorMsg),
          "Invalid number or parameters (expected {}, passed {}).",
          sizeof...(argIdx), duk_get_top(ctx)
        );
      }

      return { DUK_RET_TYPE_ERROR, DUK_ERR_TYPE_ERROR, errorMsg };
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


// Goes over each function signature.
template<bool IsPropertyCall, typename ...Signature>
struct FunctionSignatureWrapper
{
  static FuncCallResult run(duk_context* ctx, auto&& func)
  {
    FuncCallResult result;
    
    (void)(((result =
      [&]()
      {
        using ArgsTuple = boost::callable_traits::args_t<Signature>;

        static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;

        return FunctionWrapper<Signature, std::make_index_sequence<argCount>, IsPropertyCall>::run(
          ctx, std::forward<decltype(func)>(func)
        );
      }()).value < 0) && ...);

    // When function is overloaded, all errors raised by a single overload lose meaning here.
    // We overwrite the error with a generic message.
    if (sizeof...(Signature) != 1 &&
        result.value < 0)
    {
      result.errorCode = DUK_ERR_TYPE_ERROR;
      result.errorMessage = make_string(ctx, "No matching function found.");
    }

    return result;
  }
};


template<typename FuncDesc, bool IsPropertyCall = false>
struct FunctionDescWrapper;

template<bool IsPropertyCall, auto Func, typename ...Signature>
struct FunctionDescWrapper<function_descriptor<Func, Signature...>, IsPropertyCall>
{
  static FuncCallResult runImpl(duk_context* ctx)
  {
    return FunctionSignatureWrapper<IsPropertyCall, Signature...>::run(ctx, Func);
  }

  static duk_ret_t run(duk_context* ctx)
  {
    return runImpl(ctx).value;
  }
};


template<typename ...Signature>
duk_ret_t overloadedFunctionWrapper(duk_context* ctx, auto&& func)
{
  auto result = FunctionSignatureWrapper<false, Signature...>::run(ctx, std::forward<decltype(func)>(func));

  if (result.value < 0)
  {
    return throwESError(
      ctx, result.errorCode,
      result.errorMessage ? std::string_view(*result.errorMessage) :
                            "No matching function found."
    );
  }

  return result.value;
}


// Goes over each function descriptor.
template<typename ...FuncDesc>
duk_ret_t overloadedFunctionWrapper(duk_context* ctx)
{
  FuncCallResult result;

  if ((((result = FunctionDescWrapper<FuncDesc>::runImpl(ctx)).value < 0) && ...))
  {
    // When function has multiple descriptors, all errors raised for a single descriptor lose meaning here.
    // We overwrite the error with a generic message.
    if (sizeof...(FuncDesc) != 1)
    {
      result.errorCode = DUK_ERR_TYPE_ERROR;
      result.errorMessage = make_string(ctx, "No matching function found.");
    }

    return throwESError(
      ctx, result.errorCode,
      result.errorMessage ? std::string_view(*result.errorMessage) :
                            "No matching function found."
    );
  }

  return result.value;
}


} // namespace detail


} // namespace duk


#endif // DUKCPP_DETAIL_FUNCTION_WRAPPER_H
