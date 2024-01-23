#ifndef DUKCPP_DETAIL_MEMORY_RESOURCE_H
#define DUKCPP_DETAIL_MEMORY_RESOURCE_H

#include <duktape.h>
#include <memory_resource>


namespace duk::detail
{


// TODO:
// This memory_resource ignores alignment requirements, and is most likely invalid (see: LWG 2843).
// Don't use it.

class memory_resource : public std::pmr::memory_resource
{
public:
  memory_resource(duk_context* ctx) :
    ctx_(ctx)
  {
  }

private:
  [[nodiscard]]
  virtual void* do_allocate(std::size_t bytes, [[maybe_unused]] std::size_t alignment) override
  {
    return duk_alloc(ctx_, bytes);
  }

  virtual void do_deallocate(
    void* ptr, [[maybe_unused]] std::size_t bytes, [[maybe_unused]] std::size_t alignment
  ) override
  {
    duk_free(ctx_, ptr);
  }

  [[nodiscard]]
  virtual bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
  {
    auto otherPtr = dynamic_cast<const memory_resource*>(&other);
    if (!otherPtr)
      return false;

    return ctx_ == otherPtr->ctx_;
  }

  duk_context* ctx_;
};


} // namespace duk::detail


#endif // DUKCPP_DETAIL_MEMORY_RESOURCE_H
