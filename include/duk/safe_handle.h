#ifndef DUKCPP_SAFE_HANDLE_H
#define DUKCPP_SAFE_HANDLE_H

#include <duk/error.h>
#include <duk/handle.h>
#include <duktape.h>


namespace duk
{


// Owning handle to a Duktape heap object.
class safe_handle final
{
public:
  safe_handle(const handle& handle) :
    handle_(handle)
  {
    incRef();
  }

  safe_handle(const safe_handle& other) :
    handle_(other.handle_)
  {
    incRef();
  }

  safe_handle(safe_handle&& other) :
    handle_(std::move(other.handle_))
  {
  }

  // TODO: Implement if needed.
  safe_handle& operator=(const safe_handle&) = delete;
  safe_handle& operator=(safe_handle&&) = delete;

  ~safe_handle()
  {
    decRef();
  }

  const handle& get() const noexcept
  {
    return handle_;
  }

private:
  enum PropertyIndex
  {
    INFO_IDX,
    OBJECT_IDX
  };

  struct Info final
  {
    unsigned long long refCount = 1;
  };

  void incRef()
  {
    if (handle_.empty())
      return;

    duk_push_global_stash(handle_.ctx);

    if (!duk_get_prop_heapptr(handle_.ctx, -1, handle_.heap_ptr))
    {
      duk_pop(handle_.ctx); // Pop undefined.

      duk_push_bare_array(handle_.ctx);

      duk_push_heapptr(handle_.ctx, handle_.heap_ptr);
      if (!duk_put_prop_index(handle_.ctx, -2, OBJECT_IDX)) [[unlikely]]
        throw error(handle_.ctx, "handle corrupted");

      duk_push_pointer(handle_.ctx, make<Info>(handle_.ctx));
      if (!duk_put_prop_index(handle_.ctx, -2, INFO_IDX)) [[unlikely]]
        throw error(handle_.ctx, "handle corrupted");

      if (!duk_put_prop_heapptr(handle_.ctx, -2, handle_.heap_ptr)) [[unlikely]]
        throw error(handle_.ctx, "handle corrupted");
    }
    else
    {
      if (!duk_get_prop_index(handle_.ctx, -1, INFO_IDX)) [[unlikely]]
        throw error(handle_.ctx, "handle corrupted");

      auto info = static_cast<Info*>(duk_get_pointer(handle_.ctx, -1));
      ++info->refCount;

      duk_pop_2(handle_.ctx);
    }

    duk_pop(handle_.ctx); // Pop global stash.
  }

  void decRef()
  {
    if (handle_.empty())
      return;

    duk_push_global_stash(handle_.ctx);

    if (duk_get_prop_heapptr(handle_.ctx, -1, handle_.heap_ptr))
    {
      if (!duk_get_prop_index(handle_.ctx, -1, INFO_IDX)) [[unlikely]]
        throw error(handle_.ctx, "handle corrupted");

      auto info = static_cast<Info*>(duk_get_pointer(handle_.ctx, -1));

      duk_pop_2(handle_.ctx);

      if (--info->refCount == 0)
      {
        if (!duk_del_prop_heapptr(handle_.ctx, -1, handle_.heap_ptr)) [[unlikely]]
          throw error(handle_.ctx, "handle corrupted");

        free(handle_.ctx, info);
      }
    }
    else [[unlikely]]
    {
      duk_pop(handle_.ctx); // Pop undefined.
      throw error(handle_.ctx, "handle corrupted");
    }

    duk_pop(handle_.ctx); // Pop global stash.
  }

  handle handle_;
};


} // namespace duk


#endif // DUKCPP_SAFE_HANDLE_H
