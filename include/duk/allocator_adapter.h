#ifndef DUKCPP_ALLOC_ADAPTER_H
#define DUKCPP_ALLOC_ADAPTER_H

#include <duktape.h>
#include <memory>


namespace duk
{


template<typename allocator_t = std::allocator<std::byte>>
class allocator_adapter final
{
  static_assert(sizeof(typename std::allocator_traits<allocator_t>::value_type) == 1);
  static_assert(std::is_trivial_v<typename std::allocator_traits<allocator_t>::value_type>);

  struct BlockInfo final
  {
    duk_size_t size;
  };

  static_assert(std::is_trivial_v<BlockInfo>);

public:
  allocator_adapter(const allocator_t& allocator = {}) :
    allocator_(allocator)
  {
  }

  [[nodiscard]]
  static void* alloc(void* udata, duk_size_t size)
  {
    auto self = static_cast<allocator_adapter*>(udata);

    auto ptr = self->allocator_.allocate(sizeof(BlockInfo) + size);
    auto blockInfo = reinterpret_cast<BlockInfo*>(ptr);
    blockInfo->size = size;
    
    return ptr + sizeof(BlockInfo);
  }

  [[nodiscard]]
  static void* realloc(void* udata, void* ptr, duk_size_t newSize)
  {
    if (!ptr)
      return alloc(udata, newSize);

    auto oldBlockInfo = blockInfoFromPtr(ptr);

    if (newSize <= oldBlockInfo->size)
      return ptr;

    auto newPtr = alloc(udata, newSize);
    std::memcpy(newPtr, ptr, std::min(newSize, oldBlockInfo->size));
    free(udata, ptr);

    return newPtr;
  }

  static void free(void* udata, void* ptr)
  {
    if (!ptr)
      return;

    auto self = static_cast<allocator_adapter*>(udata);
    auto blockInfo = blockInfoFromPtr(ptr);

    self->allocator_.deallocate(
      reinterpret_cast<std::allocator_traits<allocator_t>::pointer>(blockInfo),
      sizeof(BlockInfo) + blockInfo->size
    );
  }

private:
  [[nodiscard]]
  static BlockInfo* blockInfoFromPtr(void* ptr)
  {
    return reinterpret_cast<BlockInfo*>(
      static_cast<std::allocator_traits<allocator_t>::pointer>(ptr) - sizeof(BlockInfo)
    );
  }

  allocator_t allocator_;
};


} // namespace duk


#endif // DUKCPP_ALLOC_ADAPTER_H
