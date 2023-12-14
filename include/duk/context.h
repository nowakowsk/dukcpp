#ifndef DUKCPP_CONTEXT_H
#define DUKCPP_CONTEXT_H

#include <duktape.h>
#include <memory>


namespace duk
{


class context final
{
public:
  context(duk_context* ctx) noexcept;

  operator duk_context*() const noexcept;

  void release() noexcept;

private:
  struct ContextDeleter final
  {
    void operator()(duk_context* ctx) const noexcept;
  };

  std::unique_ptr<duk_context, ContextDeleter> ctx_;
};


} // namespace duk


#endif // DUKCPP_CONTEXT_H
