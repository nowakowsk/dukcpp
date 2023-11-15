#ifndef DUKCPP_VALUE_H
#define DUKCPP_VALUE_H

#include <duktape.h>
#include <string>
#include <string_view>
#include <type_traits>


namespace duk
{


template<typename T>
concept integer = std::is_integral_v<T> && !std::is_same_v<T, bool>;


template<typename T>
struct type_traits;


// Treats references as values.
template<typename T> requires std::is_reference_v<T>
struct type_traits<T>
{
  using DecayT = std::decay_t<T>;

  static void push(duk_context* ctx, T value)
  {
    type_traits<DecayT>::push(ctx, value);
  }

  [[nodiscard]]
  static auto pull(duk_context* ctx, duk_idx_t idx)
  {
    return type_traits<DecayT>::pull(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return type_traits<DecayT>::check_type(ctx, idx);
  }
};


template<integer T> requires std::is_signed_v<T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_int(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_int(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<integer T> requires std::is_unsigned_v<T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_uint(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_uint(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<std::floating_point T>
struct type_traits<T>
{
  static void push(duk_context* ctx, T value)
  {
    duk_push_number(ctx, value);
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_number(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_number(ctx, idx);
  }
};


template<>
struct type_traits<bool>
{
  static void push(duk_context* ctx, bool value)
  {
    duk_push_boolean(ctx, value);
  }

  [[nodiscard]]
  static bool pull(duk_context* ctx, duk_idx_t idx)
  {
    return duk_get_boolean(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_boolean(ctx, idx);
  }
};


template<typename T> requires
  std::is_same_v<T, std::string> ||
  std::is_same_v<T, std::string_view>
struct type_traits<T>
{
  static void push(duk_context* ctx, const T& value)
  {
    duk_push_lstring(ctx, value.data(), value.size());
  }

  [[nodiscard]]
  static T pull(duk_context* ctx, duk_idx_t idx)
  {
    duk_size_t size;
    auto string = duk_get_lstring(ctx, idx, &size);

    return { string, size };
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx)
  {
    return duk_is_string(ctx, idx);
  }
};


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


} // namespace duk


#endif // DUKCPP_VALUE_H
