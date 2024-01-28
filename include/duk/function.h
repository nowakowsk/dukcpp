#ifndef DUKCPP_FUNCTION_H
#define DUKCPP_FUNCTION_H

#include <duk/common.h>
#include <duk/handle.h>
#include <duk/scoped_pop.h>
#include <duk/type_traits_helpers.h>
#include <duktape.h>


namespace duk
{


// Forward declarations
template<typename T>
struct type_traits;


template<callable T>
class function
{
public:
  function(const handle& handle) :
    handle_(handle)
  {
  }

  decltype(auto) operator()(auto&& ...args) const
  {
    using Result = boost::callable_traits::return_type_t<T>;

    duk_push_heapptr(handle_.ctx, handle_.heap_ptr);
    (push(handle_.ctx, std::forward<decltype(args)>(args)), ...);
    duk_call(handle_.ctx, sizeof...(args));

    scoped_pop _(handle_.ctx); // Pop return value.

    return check_type_and_pull<Result>(handle_.ctx, -1);
  }

private:
  handle handle_;
};


} // namespace duk


#endif // DUKCPP_FUNCTION_H
