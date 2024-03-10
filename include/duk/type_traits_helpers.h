#ifndef DUKCPP_TYPE_TRAITS_HELPERS_H
#define DUKCPP_TYPE_TRAITS_HELPERS_H

#include <duk/error.h>
#include <duktape.h>
#include <utility>


namespace duk
{


// Forward declarations
template<typename T>
struct type_traits;


template<typename T>
void push(duk_context* ctx, T&& value)
{
  type_traits<T>::push(ctx, std::forward<T>(value));
}


template<typename T>
[[nodiscard]]
decltype(auto) pull(duk_context* ctx, duk_idx_t idx)
{
  return type_traits<T>::pull(ctx, idx);
}


template<typename T>
[[nodiscard]]
decltype(auto) check_type_and_pull(duk_context* ctx, duk_idx_t idx)
{
  if (!type_traits<T>::check_type(ctx, idx))
    throw error(ctx, "unexpected type");

  return type_traits<T>::pull(ctx, idx);
}


} // namespace duk


#endif // DUKCPP_TYPE_TRAITS_HELPERS_H
