#include <duk/context.h>


namespace duk
{


context::context(duk_context* ctx) noexcept :
  ctx_(ctx)
{
}


context::operator duk_context*() const noexcept
{
  return ctx_.get();
}


void context::release() noexcept
{
  ctx_.reset();
}


void context::ContextDeleter::operator()(duk_context* ctx) const noexcept
{
  duk_destroy_heap(ctx);
}


} // namespace duk
