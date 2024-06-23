#ifndef DUKCPP_HANDLE_H
#define DUKCPP_HANDLE_H

#include <duktape.h>


namespace duk
{


// Non-owning handle to a Duktape heap object.
struct handle final
{
  handle() noexcept = default;

  handle(duk_context* ctx, void* heap_ptr) noexcept :
    ctx(ctx),
    heap_ptr(heap_ptr)
  {
  }

  handle(duk_context* ctx, duk_idx_t idx) :
    ctx(ctx),
    heap_ptr(duk_get_heapptr(ctx, idx))
  {
  }

  ~handle() = default;

  handle(const handle&) = default;

  handle(handle&& other) :
    ctx(other.ctx),
    heap_ptr(other.heap_ptr)
  {
    other.ctx = nullptr;
    other.heap_ptr = nullptr;
  }

  handle& operator=(const handle&) = default;

  handle& operator=(handle&& other)
  {
    if (&other != this)
    {
      ctx = other.ctx;
      heap_ptr = other.heap_ptr;

      other.ctx = nullptr;
      other.heap_ptr = nullptr;
    }

    return *this;
  }

  [[nodiscard]]
  bool empty() const noexcept
  {
    return !ctx || !heap_ptr;
  }

  duk_context* ctx = nullptr;
  void* heap_ptr = nullptr;
};


} // namespace duk


#endif // DUKCPP_HANDLE_H
