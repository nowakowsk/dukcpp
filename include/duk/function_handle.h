#ifndef DUKCPP_FUNCTION_HANDLE_H
#define DUKCPP_FUNCTION_HANDLE_H

#include <duk/common.h>
#include <duk/handle.h>
#include <duk/safe_handle.h>
#include <duk/scoped_pop.h>
#include <duk/type_traits_helpers.h>
#include <duktape.h>


namespace duk
{


// Non-owning handle to Duktape function.
template<typename T, typename Handle = handle>
class function_handle
{
public:
  function_handle(const Handle& handle) :
    handle_(handle)
  {
  }

  decltype(auto) operator()(auto&& ...args) const
  {
    using Result = boost::callable_traits::return_type_t<T>;

    push_handle(handle_);
    (push(handle_.ctx(), std::forward<decltype(args)>(args)), ...);

    scoped_pop _(handle_.ctx()); // duk_call
    duk_call(handle_.ctx(), sizeof...(args));

    return safe_get<Result>(handle_.ctx(), -1);
  }

private:
  Handle handle_;
};


// Owning handle to Duktape function.
template<typename T>
using safe_function_handle = function_handle<T, safe_handle>;


template<typename T, typename ...Ts>
struct callable_traits_type<function_handle<T, Ts...>>
{
  using type = T;
};


} // namespace duk


#endif // DUKCPP_FUNCTION_HANDLE_H
