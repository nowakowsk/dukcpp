#ifndef DUKCPP_CONTEXT_H
#define DUKCPP_CONTEXT_H

#include <duktape.h>
#include <memory>


namespace duk
{


namespace detail
{

struct context_deleter final
{
  void operator()(duk_context* ctx) noexcept
  {
    duk_destroy_heap(ctx);
  }
};

} // namespace detail


using context = std::unique_ptr<duk_context, detail::context_deleter>;


} // namespace duk


#endif // DUKCPP_CONTEXT_H
