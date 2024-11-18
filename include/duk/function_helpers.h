#ifndef DUKCPP_FUNCTION_HELPERS_H
#define DUKCPP_FUNCTION_HELPERS_H

#include <duk/detail/function_wrapper.h>
#include <duk/detail/type_traits.h>
#include <duktape.h>
#include <string_view>
#include <utility>


namespace duk
{


template<typename ...FuncDesc>
duk_idx_t push_function(duk_context* ctx)
{
  static constexpr auto funcWrapper = detail::overloadedFunctionWrapper<FuncDesc...>;

  return duk_push_c_function(ctx, funcWrapper, DUK_VARARGS);
}


template<auto ...Funcs>
duk_idx_t push_function(duk_context* ctx)
{
  return push_function<function_descriptor<Funcs, decltype(Funcs)>...>(ctx);
}


template<typename ...FuncDesc>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<FuncDesc...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<auto ...Funcs>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<Funcs...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<typename Func>
void push_function(duk_context* ctx, Func&& func)
{
  detail::type_traits<as_function<Func>>::push(ctx, std::forward<Func>(func));
}


template<typename Func>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name, Func&& func)
{
  push_function(ctx, std::forward<Func>(func));
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


} // namespace duk


#endif // DUKCPP_FUNCTION_HELPERS_H
