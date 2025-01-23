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


namespace detail
{

template<typename T>
struct type_traits;

} // namespace detail


template<typename T>
concept integer =
  std::is_integral_v<std::decay_t<T>> &&
  !std::is_same_v<std::decay_t<T>, bool> &&
  !std::is_member_pointer_v<std::decay_t<T>>;


template<typename T>
concept floating_point =
  std::is_floating_point_v<std::decay_t<T>> &&
  !std::is_member_pointer_v<std::decay_t<T>>;


template<typename T>
concept enumeration = 
  std::is_enum_v<std::decay_t<T>> &&
  !std::is_member_pointer_v<std::decay_t<T>>;


template<typename T>
concept boolean =
  std::is_same_v<std::decay_t<T>, bool> &&
  !std::is_member_pointer_v<std::decay_t<T>>;


template<typename T>
concept object = requires
{
  requires detail::type_traits<T>::detail_is_object;
};


namespace detail
{


template<typename T>
[[nodiscard]]
auto* make(duk_context* ctx, auto&&... args)
{
  auto objBuffer = allocator<T>(ctx).allocate(1);
  if (!objBuffer) [[unlikely]]
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


template<typename T>
struct DtorDeleter final
{
  void operator()(T* ptr) const
  {
    if constexpr (std::is_destructible_v<T>)
      ptr->~T();
  }
};


// expand_pack

template<typename Tuple>
struct expand_pack;

template<template<typename...> typename Tuple, typename ...Ts>
struct expand_pack<Tuple<Ts...>>
{
  static decltype(auto) run(auto&& func, auto&& ...params)
  {
    return func.template operator()<Ts...>(std::forward<decltype(params)>(params)...);
  }
};


// Duktape helpers

inline static bool get_prop_string(duk_context *ctx, duk_idx_t idx, std::string_view name)
{
  return duk_get_prop_lstring(ctx, idx, name.data(), name.length());
}


inline static bool put_prop_string(duk_context *ctx, duk_idx_t idx, std::string_view name)
{
  return duk_put_prop_lstring(ctx, idx, name.data(), name.length());
}


inline static bool del_prop_string(duk_context *ctx, duk_idx_t idx, std::string_view name)
{
  return duk_del_prop_lstring(ctx, idx, name.data(), name.length());
}


} // namespace detail


} // namespace duk


#endif // DUKCPP_COMMON_H
