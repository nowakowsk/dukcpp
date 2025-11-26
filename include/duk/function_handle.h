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
template<typename Result, typename Handle = handle>
class function_handle
{
public:
  function_handle(const Handle& handle) :
    handle_(handle)
  {
  }

  decltype(auto) operator()(auto&& ...args) const
  {
    auto ctx = handle_.ctx();

    push_handle(handle_);
    (push(ctx, std::forward<decltype(args)>(args)), ...);

    scoped_pop _(ctx); // duk_pcall
    if (duk_pcall(ctx, sizeof...(args)) != DUK_EXEC_SUCCESS)
      throw duk::es_error(ctx, -1);

    return safe_get<Result>(ctx, -1);
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
