#ifndef DUKCPP_HANDLE_H
#define DUKCPP_HANDLE_H

#include <duktape.h>
#include <concepts>


namespace duk
{


// Non-owning handle to a Duktape heap object.
struct handle final
{
  handle() noexcept = default;

  handle(duk_context* ctx, void* heap_ptr) noexcept :
    ctx_(ctx),
    heap_ptr_(heap_ptr)
  {
  }

  handle(duk_context* ctx, duk_idx_t idx) noexcept :
    ctx_(ctx),
    heap_ptr_(duk_get_heapptr(ctx, idx))
  {
  }

  ~handle() noexcept = default;

  handle(const handle& other) noexcept :
    ctx_(other.ctx_),
    heap_ptr_(other.heap_ptr_)
  {
  }

  handle(handle&& other) noexcept :
    ctx_(other.ctx_),
    heap_ptr_(other.heap_ptr_)
  {
    other.ctx_ = nullptr;
    other.heap_ptr_ = nullptr;
  }

  handle& operator=(const handle& other) noexcept
  {
    if (&other != this)
    {
      ctx_ = other.ctx_;
      heap_ptr_ = other.heap_ptr_;
    }

    return *this;
  }

  handle& operator=(handle&& other) noexcept
  {
    if (&other != this)
    {
      ctx_ = other.ctx_;
      heap_ptr_ = other.heap_ptr_;

      other.ctx_ = nullptr;
      other.heap_ptr_ = nullptr;
    }

    return *this;
  }

  [[nodiscard]]
  bool operator==(const handle& other) const noexcept
  {
    return ctx_ == other.ctx_ &&
           heap_ptr_ == other.heap_ptr_;
  }

  [[nodiscard]]
  bool operator!=(const handle& other) const noexcept
  {
    return !operator==(other);
  }

  [[nodiscard]]
  bool empty() const noexcept
  {
    return !ctx_ || !heap_ptr_;
  }

  [[nodiscard]]
  duk_context* ctx() const noexcept
  {
    return ctx_;
  }

  [[nodiscard]]
  void* heap_ptr() const noexcept
  {
    return heap_ptr_;
  }

private:
  duk_context* ctx_ = nullptr;
  void* heap_ptr_ = nullptr;
};


template<typename T>
concept handle_type =
  std::regular<T> &&
  std::constructible_from<T, const handle&> &&
  std::assignable_from<T&, const handle&> &&
  requires(const T& handle)
  {
    { handle.ctx() } -> std::same_as<duk_context*>;
    { handle.heap_ptr() } -> std::same_as<void*>;
    { handle.empty() } -> std::same_as<bool>;
  };


void push_handle(const handle_type auto& handle) noexcept
{
  duk_push_heapptr(handle.ctx(), handle.heap_ptr());
}


} // namespace duk


#endif // DUKCPP_HANDLE_H
