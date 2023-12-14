#ifndef DUKCPP_COMMON_H
#define DUKCPP_COMMON_H

#include <duk/error.h>
#include <boost/callable_traits.hpp>
#include <duktape.h>
#include <type_traits>
#include <utility>


#define DUKCPP_DETAIL_INTERNAL_NAME(name) ("\xff\xff" "dukcpp_" name)


namespace duk
{


template<typename T>
concept integer = std::is_integral_v<T> && !std::is_same_v<T, bool>;


template<typename T>
concept callable =
  !std::is_reference_v<T> &&
  requires(T)
  {
    typename boost::callable_traits::function_type_t<T>;
  };


template<typename T>
auto alloc(duk_context* ctx, auto&&... args)
{
  auto objBuffer = duk_alloc(ctx, sizeof(T));

  if (!objBuffer)
    throw error("Memory allocation failed.");

  return new (objBuffer) T(std::forward<decltype(args)>(args)...);
}


template<typename T>
void free(duk_context* ctx, T* ptr)
{
  if (!ptr)
    return;

  if constexpr (std::is_destructible_v<T>)
    ptr->~T();

  duk_free(ctx, ptr);
}


} // namespace duk


#endif // DUKCPP_COMMON_H
