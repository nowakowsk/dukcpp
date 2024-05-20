#ifndef DUKCPP_STRING_TRAITS_H
#define DUKCPP_STRING_TRAITS_H

#include <duktape.h>
#include <string>
#include <string_view>
#include <utility>


namespace duk
{


template<typename T>
struct string_traits;


template<typename T>
concept string_type = requires(T str, const char* char_str, duk_size_t size)
{
  { string_traits<std::decay_t<T>>::make_string(char_str, size) } -> std::same_as<std::decay_t<T>>;
  { string_traits<std::decay_t<T>>::make_view(str) } -> std::same_as<std::string_view>;
};


namespace detail
{


template<typename T>
struct std_string_traits_impl
{
  [[nodiscard]]
  static T make_string(const char* str, duk_size_t size)
  {
    return { str, size };
  }

  [[nodiscard]]
  static std::string_view make_view(const T& str)
  {
    return str;
  }
};


} // namespace detail


// std::string
template<typename ...Args>
struct string_traits<std::basic_string<char, Args...>> :
  detail::std_string_traits_impl<std::basic_string<char, Args...>>
{
};


// std::string_view
template<typename ...Args>
struct string_traits<std::basic_string_view<char, Args...>> :
  detail::std_string_traits_impl<std::basic_string_view<char, Args...>>
{
};


// const char*
template<>
struct string_traits<const char*>
{
  [[nodiscard]]
  static const char* make_string(const char* str, [[maybe_unused]] duk_size_t size)
  {
    return str;
  }

  [[nodiscard]]
  static std::string_view make_view(const char* str)
  {
    return str;
  }
};


} // namespace duk


#endif // DUKCPP_STRING_TRAITS_H
