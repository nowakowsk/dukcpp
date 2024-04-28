#ifndef DUKCPP_FUNCTION_HELPERS_H
#define DUKCPP_FUNCTION_HELPERS_H

#include <duk/type_traits.h>
#include <duk/detail/function_wrapper.h>
#include <duktape.h>
#include <string_view>
#include <utility>


namespace duk
{


template<auto ...funcs>
duk_idx_t push_function(duk_context* ctx)
{
  static constexpr auto funcWrapper = detail::overloadedFunctionWrapper<funcs...>;

  return duk_push_c_function(ctx, funcWrapper, DUK_VARARGS);
}


template<auto ...funcs>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<funcs...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<typename func_t>
void push_function(duk_context* ctx, func_t&& func)
{
  type_traits<as_function<func_t>>::push(ctx, std::forward<func_t>(func));
}


template<typename func_t>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name, func_t&& func)
{
  push_function(ctx, std::forward<func_t>(func));
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


} // namespace duk


#endif // DUKCPP_FUNCTION_HELPERS_H
