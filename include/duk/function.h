#ifndef DUKCPP_FUNCTION_H
#define DUKCPP_FUNCTION_H

#include <duk/common.h>
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
  function(duk_context* ctx, void* heap_ptr) :
    ctx_(ctx),
    heapPtr_(heap_ptr)
  {
  }

  decltype(auto) operator()(auto&& ...args) const
  {
    using Result = boost::callable_traits::return_type_t<T>;

    duk_push_heapptr(ctx_, heapPtr_);
    (push(ctx_, std::forward<decltype(args)>(args)), ...);
    duk_call(ctx_, sizeof...(args));

    scoped_pop _(ctx_); // Pop return value.

    return check_type_and_pull<Result>(ctx_, -1);
  }

private:
  duk_context* ctx_;
  void* heapPtr_;
};


} // namespace duk


#endif // DUKCPP_FUNCTION_H
