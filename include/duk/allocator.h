#ifndef DUKCPP_ALLOCATOR_H
#define DUKCPP_ALLOCATOR_H

#include <duktape.h>
#include <type_traits>


namespace duk
{


template<typename T>
class allocator
{
public:
  using value_type = T;
  using is_always_equal = std::false_type;

  allocator(duk_context* ctx) noexcept :
    ctx_(ctx)
  {
  }

  template<typename U>
  allocator(const allocator<U>& other) noexcept :
    ctx_(other.ctx_)
  {
  }

  template<typename U>
  allocator(allocator<U>&& other) noexcept :
    ctx_(other.ctx_)
  {
  }

  [[nodiscard]]
  bool operator==(const allocator& other) const noexcept
  {
    return ctx_ == other.ctx_;
  }

  [[nodiscard]]
  bool operator!=(const allocator& other) const noexcept
  {
    return !(*this == other);
  }

  [[nodiscard]]
  T* allocate(std::size_t n)
  {
    return static_cast<T*>(duk_alloc(ctx_, sizeof(T) * n));
  }

  void deallocate(T* ptr, [[maybe_unused]] std::size_t n) noexcept
  {
    duk_free(ctx_, ptr);
  }

  // Can't be private because of generic copy/move c-tors.
  duk_context* ctx_;
};


} // namespace duk


#endif // DUKCPP_ALLOCATOR_H
