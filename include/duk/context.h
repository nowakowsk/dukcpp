#ifndef DUKCPP_CONTEXT_H
#define DUKCPP_CONTEXT_H

#include <duktape.h>
#include <memory>


namespace duk
{


class context final
{
public:
  context(duk_context* ctx) noexcept :
    ctx_(ctx)
  {
  }

  operator duk_context*() const noexcept
  {
    return ctx_.get();
  }

  void release() noexcept
  {
    ctx_.reset();
  }

private:
  struct ContextDeleter final
  {
    void operator()(duk_context* ctx) const noexcept
    {
      duk_destroy_heap(ctx);
    }
  };

  std::unique_ptr<duk_context, ContextDeleter> ctx_;
};


} // namespace duk


#endif // DUKCPP_CONTEXT_H
