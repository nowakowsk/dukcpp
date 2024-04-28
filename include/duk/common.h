#ifndef DUKCPP_COMMON_H
#define DUKCPP_COMMON_H

#include <duk/allocator.h>
#include <duk/error.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <utility>


#define DUKCPP_DETAIL_INTERNAL_NAME(name) ("\xff\xff" "dukcpp_" name)


namespace duk
{


template<typename T>
concept integer = 
  std::is_integral_v<T> &&
  !std::is_same_v<std::remove_cv_t<T>, bool>;


template<typename T>
[[nodiscard]]
auto* make(duk_context* ctx, auto&&... args)
{
  auto objBuffer = allocator<T>(ctx).allocate(1);
  if (!objBuffer)
    throw error(ctx, "Memory allocation failed.");

  return new (objBuffer) T(std::forward<decltype(args)>(args)...);
}


template<typename T>
void free(duk_context* ctx, T* ptr)
{
  if (!ptr)
    return;

  if constexpr (std::is_destructible_v<T>)
    ptr->~T();

  allocator<T>(ctx).deallocate(ptr, 1);
}


} // namespace duk


#endif // DUKCPP_COMMON_H
