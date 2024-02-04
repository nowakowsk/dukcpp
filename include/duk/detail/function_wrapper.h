#ifndef DUKCPP_DETAIL_FUNCTION_WRAPPER_H
#define DUKCPP_DETAIL_FUNCTION_WRAPPER_H

#include <duk/type_traits.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <functional>
#include <tuple>
#include <utility>


namespace duk::detail
{


template<typename Func, typename ArgIdx>
struct FunctionWrapper;

template<typename Func, std::size_t ...argIdx>
struct FunctionWrapper<Func, std::index_sequence<argIdx...>>
{
  using Result = boost::callable_traits::return_type_t<Func>;
  using ArgsTuple = boost::callable_traits::args_t<Func>;

  static duk_ret_t run(duk_context* ctx, auto&& func)
  {
    bool argsMatch = duk_get_top(ctx) == sizeof...(argIdx) &&
                     (type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::check_type(ctx, argIdx) && ...);
    if (!argsMatch)
      return DUK_RET_TYPE_ERROR;

    if constexpr (std::is_same_v<Result, void>)
    {
      std::invoke(func, type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::pull(ctx, argIdx)...);
      return 0;
    }
    else
    {
      type_traits<Result>::push(
        ctx,
        std::invoke(func, type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::pull(ctx, argIdx)...)
      );
      return 1;
    }
  }
};


template<auto func>
duk_ret_t functionWrapper(duk_context* ctx)
{
  using ArgsTuple = boost::callable_traits::args_t<decltype(func)>;

  static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;

  return FunctionWrapper<decltype(func), std::make_index_sequence<argCount>>::run(ctx, func);
}


template<auto ...funcs>
duk_ret_t overloadedFunctionWrapper(duk_context* ctx)
{
  duk_ret_t result;

  if ((((result = functionWrapper<funcs>(ctx)) < 0) && ...))
    return duk_error(ctx, DUK_ERR_TYPE_ERROR, "No matching function overload found.");

  return result;
}


} // namespace duk::detail


#endif // DUKCPP_DETAIL_FUNCTION_WRAPPER_H
