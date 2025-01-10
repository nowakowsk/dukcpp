#ifndef DUKCPP_FUNCTION_HELPERS_H
#define DUKCPP_FUNCTION_HELPERS_H

#include <duk/detail/function_wrapper.h>
#include <duk/detail/type_traits.h>
#include <duktape.h>
#include <string_view>
#include <utility>


namespace duk
{


namespace detail
{


// to_method_signature

template<typename Signature>
struct to_method_signature
{
  using type = Signature;
};

template<typename Signature>
requires(!requires { typename boost::callable_traits::class_of_t<Signature>; })
struct to_method_signature<Signature>
{
  template<typename Args>
  struct expand_args;

  template<typename Class, typename ...Args>
  struct expand_args<std::tuple<Class, Args...>>
  {
    using Result = boost::callable_traits::return_type_t<Signature>;
    using DecayClass = std::decay_t<Class>;
    using type = Result(DecayClass::*)(Args...);
  };

  using ArgsTuple = boost::callable_traits::args_t<Signature>;
  using type = expand_args<ArgsTuple>::type;
};

template<typename Signature>
using to_method_signature_t = to_method_signature<Signature>::type;


} // namespace detail


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


template<auto ...Func>
duk_idx_t push_method(duk_context* ctx)
{
  return push_function<function_descriptor<Func, detail::to_method_signature_t<decltype(Func)>>...>(ctx);
}


template<typename ...FuncDesc>
void put_prop_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<FuncDesc...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<auto ...Func>
void put_prop_function(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_function<Func...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<auto ...Func>
void put_prop_method(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  push_method<Func...>(ctx);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<typename ...Signature>
void push_function(duk_context* ctx, auto&& func)
{
  using Func = decltype(func);

  detail::type_traits<as_function<Func>>::template push<Signature...>(ctx, std::forward<Func>(func));
}


template<typename ...Signature>
void push_method(duk_context* ctx, auto&& func)
{
  push_function<detail::to_method_signature_t<Signature>...>(ctx, std::forward<decltype(func)>(func));
}


template<typename ...Signature>
void put_prop_function(duk_context* ctx, duk_idx_t idx, std::string_view name, auto&& func)
{
  push_function<Signature...>(ctx, std::forward<decltype(func)>(func));
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


template<typename ...Signature>
void put_prop_method(duk_context* ctx, duk_idx_t idx, std::string_view name, auto&& func)
{
  push_method<Signature...>(ctx, std::forward<decltype(func)>(func));
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
}


} // namespace duk


#endif // DUKCPP_FUNCTION_HELPERS_H
