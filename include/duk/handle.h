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

  handle(duk_context* ctx, duk_idx_t idx) noexcept :
    ctx(ctx),
    heap_ptr(duk_get_heapptr(ctx, idx))
  {
  }

  ~handle() noexcept = default;

  handle(const handle& other) noexcept :
    ctx(other.ctx),
    heap_ptr(other.heap_ptr)
  {
  }

  handle(handle&& other) noexcept :
    ctx(other.ctx),
    heap_ptr(other.heap_ptr)
  {
    other.ctx = nullptr;
    other.heap_ptr = nullptr;
  }

  handle& operator=(const handle& other) noexcept
  {
    if (&other != this)
    {
      ctx = other.ctx;
      heap_ptr = other.heap_ptr;
    }

    return *this;
  }

  handle& operator=(handle&& other) noexcept
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
  bool operator==(const handle& other) const noexcept
  {
    return ctx == other.ctx &&
           heap_ptr == other.heap_ptr;
  }

  [[nodiscard]]
  bool operator!=(const handle& other) const noexcept
  {
    return !operator==(other);
  }

  [[nodiscard]]
  bool empty() const noexcept
  {
    return !ctx || !heap_ptr;
  }

  void push() const noexcept
  {
    duk_push_heapptr(ctx, heap_ptr);
  }

  duk_context* ctx = nullptr;
  void* heap_ptr = nullptr;
};


} // namespace duk


#endif // DUKCPP_HANDLE_H
