#ifndef DUKCPP_TYPE_TRAITS_HELPERS_H
#define DUKCPP_TYPE_TRAITS_HELPERS_H

#include <duk/error.h>
#include <duk/fwd.h>
#include <duktape.h>
#include <utility>


namespace duk
{


template<typename T>
[[nodiscard]]
bool check_type(duk_context* ctx, duk_idx_t idx) noexcept
{
  return detail::type_traits<T>::check_type(ctx, idx);
}


template<typename T>
void push(duk_context* ctx, T&& value, auto&&... args)
{
  detail::type_traits<T>::push(ctx, std::forward<T>(value), std::forward<decltype(args)>(args)...);
}


template<typename T>
void put_prop(duk_context* ctx, duk_idx_t idx, std::string_view name, T&& value, auto&&... args)
{
  push(ctx, std::forward<T>(value), std::forward<decltype(args)>(args)...);
  duk_put_prop_lstring(ctx, idx - 1, name.data(), name.length());
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


inline static bool finalize(duk_context* ctx, duk_idx_t idx)
{
  return detail::finalize_object(ctx, idx) ||
         detail::finalize_callable(ctx, idx);
}


} // namespace duk


#endif // DUKCPP_TYPE_TRAITS_HELPERS_H
