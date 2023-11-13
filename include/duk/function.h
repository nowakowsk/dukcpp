#ifndef DUKCPP_FUNCTION_H
#define DUKCPP_FUNCTION_H

#include <duk/value.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <functional>
#include <string_view>
#include <tuple>
#include <utility>


namespace duk
{


namespace detail
{


template<auto func, typename ArgIdx>
struct FunctionWrapper;

template<auto func, std::size_t ...argIdx>
struct FunctionWrapper<func, std::index_sequence<argIdx...>>
{
  using Result = boost::callable_traits::return_type_t<decltype(func)>;
  using ArgsTuple = boost::callable_traits::args_t<decltype(func)>;

  static duk_ret_t run(duk_context* ctx)
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
      auto result = std::invoke(func, type_traits<std::tuple_element_t<argIdx, ArgsTuple>>::pull(ctx, argIdx) ...);
      type_traits<Result>::push(ctx, result);
      return 1;
    }
  }
};

template<auto ...funcs>
duk_ret_t overloadedFunctionWrapper(duk_context* ctx)
{
  duk_ret_t result;

  (void)(
    ((result = 
        [&]()
        {
          using ArgsTuple = boost::callable_traits::args_t<decltype(funcs)>;

          static constexpr auto argCount = std::tuple_size_v<ArgsTuple>;

          return FunctionWrapper<funcs, std::make_index_sequence<argCount>>::run(ctx);
        }()) >= 0) || ... 
  );

  return result;
}


} // namespace detail


template<auto ...funcs>
void register_function(duk_context* ctx, duk_idx_t obj_idx, std::string_view name)
{
  static constexpr auto funcWrapper = detail::overloadedFunctionWrapper<funcs...>;

  duk_push_c_function(ctx, funcWrapper, DUK_VARARGS);
  duk_put_prop_lstring(ctx, obj_idx - 1, name.data(), name.length());
}


} // namespace duk


#endif // DUKCPP_FUNCTION_H
