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


template<auto ...Func>
duk_idx_t push_function(duk_context* ctx)
{
  return push_function<function_descriptor<Func, decltype(Func)>...>(ctx);
}


// TODO: Rename to duk::put_prop_function
template<typename ...FuncDesc>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<FuncDesc...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<auto ...Func>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<Func...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<typename ...Signature>
void push_function(duk_context* ctx, auto&& func)
{
  using Func = decltype(func);

  detail::type_traits<as_function<Func>>::template push<Signature...>(ctx, std::forward<Func>(func));
}


template<typename ...Signature>
void put_function(duk_context* ctx, duk_idx_t idx, std::string_view name, auto&& func)
{
  push_function<Signature...>(ctx, std::forward<decltype(func)>(func));
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


} // namespace duk


#endif // DUKCPP_FUNCTION_HELPERS_H
