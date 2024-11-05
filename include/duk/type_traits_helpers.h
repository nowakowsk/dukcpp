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


// TODO: Make sure it never throws.
template<typename T>
[[nodiscard]]
bool check_type(duk_context* ctx, duk_idx_t idx)
{
  return detail::type_traits<T>::check_type(ctx, idx);
}


template<typename T>
void push(duk_context* ctx, T&& value, auto&&... args)
{
  detail::type_traits<T>::push(ctx, std::forward<T>(value), std::forward<decltype(args)>(args)...);
}


template<typename T>
[[nodiscard]]
decltype(auto) get(duk_context* ctx, duk_idx_t idx)
{
  return detail::type_traits<T>::get(ctx, idx);
}


// Checks type before getting. Throws error if check fails.
template<typename T>
[[nodiscard]]
decltype(auto) safe_get(duk_context* ctx, duk_idx_t idx)
{
  if (!check_type<T>(ctx, idx)) [[unlikely]]
    throw error(ctx, "unexpected type");

  return get<T>(ctx, idx);
}


} // namespace duk


#endif // DUKCPP_TYPE_TRAITS_HELPERS_H
