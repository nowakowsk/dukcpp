#ifndef DUKCPP_SAFE_HANDLE_H
#define DUKCPP_SAFE_HANDLE_H

#include <duk/common.h>
#include <duk/error.h>
#include <duk/handle.h>
#include <duk/scoped_pop.h>
#include <utility>


namespace duk
{


// Owning handle to a Duktape heap object.
class safe_handle final
{
public:
  safe_handle() noexcept = default;

  safe_handle(const handle& handle) noexcept :
    handle_(handle)
  {
    incRef();
  }

  safe_handle(const safe_handle& other) noexcept :
    handle_(other.handle_)
  {
    incRef();
  }

  safe_handle(safe_handle&& other) noexcept :
    handle_(std::move(other.handle_))
  {
  }

  safe_handle& operator=(const safe_handle& other) noexcept
  {
    if (&other != this)
    {
      decRef();
      handle_ = other.handle_;
      incRef();
    }

    return *this;
  }

  safe_handle& operator=(safe_handle&& other) noexcept
  {
    if (&other != this)
    {
      decRef();
      handle_ = std::move(other.handle_);
    }

    return *this;
  }

  ~safe_handle() noexcept
  {
    decRef();
  }

  [[nodiscard]]
  bool operator==(const safe_handle& other) const noexcept
  {
    return handle_ == other.handle_;
  }

  [[nodiscard]]
  bool operator!=(const safe_handle& other) const noexcept
  {
    return !operator==(other);
  }

  [[nodiscard]]
  const handle& get() const noexcept
  {
    return handle_;
  }

  [[nodiscard]]
  bool empty() const noexcept
  {
    return handle_.empty();
  }

  [[nodiscard]]
  duk_context* ctx() const noexcept
  {
    return handle_.ctx();
  }

  [[nodiscard]]
  void* heap_ptr() const noexcept
  {
    return handle_.heap_ptr();
  }

private:
  static constexpr duk_size_t heapPtrSize = sizeof(std::declval<duk::handle>().heap_ptr());

  enum PropertyIndex
  {
    INFO_IDX,
    OBJECT_IDX
  };

  struct Info final
  {
    unsigned long long refCount = 1;
  };

  void incRef() noexcept
  {
    if (handle_.empty())
      return;

    auto ctx = handle_.ctx();
    auto heapPtr = handle_.heap_ptr();

    scoped_pop _(ctx); // duk_push_global_stash
    duk_push_global_stash(ctx);

    scoped_pop __(ctx); // duk_get_prop_heapptr
    if (!duk_get_prop_lstring(ctx, -1, reinterpret_cast<const char*>(&heapPtr), heapPtrSize))
    {
      // undefined on stack

      duk_push_bare_array(ctx);

      push_handle(handle_);
      if (!duk_put_prop_index(ctx, -2, OBJECT_IDX)) [[unlikely]]
        duk_fatal(ctx, "handle corrupted (incRef, OBJECT_IDX)");

      duk_push_pointer(ctx, detail::make<Info>(ctx));
      if (!duk_put_prop_index(ctx, -2, INFO_IDX)) [[unlikely]]
        duk_fatal(ctx, "handle corrupted (incRef, INFO_IDX)");

      if (!duk_put_prop_lstring(ctx, -3, reinterpret_cast<const char*>(&heapPtr), heapPtrSize)) [[unlikely]]
        duk_fatal(ctx, "handle corrupted (incRef, put object info)");
    }
    else
    {
      scoped_pop _(ctx); // duk_get_prop_index
      if (!duk_get_prop_index(ctx, -1, INFO_IDX)) [[unlikely]]
        duk_fatal(ctx, "handle corrupted (incRef, get object info)");

      auto info = static_cast<Info*>(duk_get_pointer(ctx, -1));
      ++info->refCount;
    }
  }

  void decRef() noexcept
  {
    if (handle_.empty())
      return;

    auto ctx = handle_.ctx();
    auto heapPtr = handle_.heap_ptr();

    scoped_pop _(ctx); // duk_push_global_stash
    duk_push_global_stash(ctx);

    scoped_pop __(ctx); // duk_get_prop_heapptr
    if (!duk_get_prop_lstring(ctx, -1, reinterpret_cast<const char*>(&heapPtr), heapPtrSize))  [[unlikely]]
      duk_fatal(ctx, "handle corrupted (decRef, get object info)");

    scoped_pop ___(ctx); // duk_get_prop_index
    if (!duk_get_prop_index(ctx, -1, INFO_IDX)) [[unlikely]]
      duk_fatal(ctx, "handle corrupted (decRef, INFO_IDX)");

    auto info = static_cast<Info*>(duk_get_pointer(ctx, -1));

    if (--info->refCount == 0)
    {
      if (!duk_del_prop_lstring(ctx, -3, reinterpret_cast<const char*>(&heapPtr), heapPtrSize)) [[unlikely]]
        duk_fatal(ctx, "handle corrupted (decRef, del object info)");

      detail::free(ctx, info);
    }
  }

  handle handle_;
};


} // namespace duk


#endif // DUKCPP_SAFE_HANDLE_H
