#ifndef DUKCPP_TYPE_TRAITS_HELPERS_H
#define DUKCPP_TYPE_TRAITS_HELPERS_H

#include <duk/error.h>
#include <duktape.h>
#include <utility>


namespace duk
{


namespace detail
{

template<typename T>
struct type_traits;

} // namespace detail


template<typename T>
void push(duk_context* ctx, T&& value, auto&&... args)
{
  detail::type_traits<T>::push(ctx, std::forward<T>(value), std::forward<decltype(args)>(args)...);
}


template<typename T>
[[nodiscard]]
decltype(auto) pull(duk_context* ctx, duk_idx_t idx)
{
  return detail::type_traits<T>::pull(ctx, idx);
}


// Checks type before pulling. Throws error if check fails.
template<typename T>
[[nodiscard]]
decltype(auto) safe_pull(duk_context* ctx, duk_idx_t idx)
{
  if (!detail::type_traits<T>::check_type(ctx, idx))
    throw error(ctx, "unexpected type");

  return detail::type_traits<T>::pull(ctx, idx);
}


} // namespace duk


#endif // DUKCPP_TYPE_TRAITS_HELPERS_H
