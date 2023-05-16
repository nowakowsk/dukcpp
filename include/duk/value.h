#ifndef DUKCPP_VALUE_H
#define DUKCPP_VALUE_H

#include <duktape.h>
#include <string_view>


namespace duk
{


template<typename T>
struct type_traits;


template<>
struct type_traits<int>
{
  static void push(duk_context* ctx, int value) noexcept
  {
    duk_push_int(ctx, value);
  }

  [[nodiscard]]
  static int pull(duk_context* ctx, duk_idx_t idx) noexcept
  {
    return duk_get_int(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx) noexcept
  {
    return duk_is_number(ctx, idx);
  }
};


template<>
struct type_traits<double>
{
  static void push(duk_context* ctx, double value) noexcept
  {
    duk_push_number(ctx, value);
  }

  [[nodiscard]]
  static double pull(duk_context* ctx, duk_idx_t idx) noexcept
  {
    return duk_get_number(ctx, idx);
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx) noexcept
  {
    return duk_is_number(ctx, idx);
  }
};


template<>
struct type_traits<std::string_view>
{
  static void push(duk_context* ctx, std::string_view value) noexcept
  {
    duk_push_lstring(ctx, value.data(), value.size());
  }

  [[nodiscard]]
  static std::string_view pull(duk_context* ctx, duk_idx_t idx)
  {
    duk_size_t size;
    auto string = duk_get_lstring(ctx, idx, &size);

    return { string, size };
  }

  [[nodiscard]]
  static bool check_type(duk_context* ctx, duk_idx_t idx) noexcept
  {
    return duk_is_string(ctx, idx);
  }
};


template<typename T>
void push(duk_context* ctx, T&& value)
{
  type_traits<T>::push(ctx, value);
}


template<typename T>
[[nodiscard]]
auto pull(duk_context* ctx, duk_idx_t idx)
{
  return type_traits<T>::pull(ctx, idx);
}


} // namespace duk


#endif // DUKCPP_VALUE_H
