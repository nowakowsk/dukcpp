#ifndef DUKCPP_STRING_TRAITS_H
#define DUKCPP_STRING_TRAITS_H

#include <duktape.h>
#include <cstring>
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
  { string_traits<T>::make(char_str, size) } -> std::same_as<T>;
  { string_traits<T>::info(str) } -> std::same_as<std::pair<const char*, duk_size_t>>;
};


namespace detail
{


template<typename T>
struct std_string_traits_impl
{
  [[nodiscard]]
  static T make(const char* str, duk_size_t size)
  {
    return { str, size };
  }

  [[nodiscard]]
  static std::pair<const char*, duk_size_t> info(const T& str)
  {
    return { str.data(), str.size() };
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
  static const char* make(const char* str, [[maybe_unused]] duk_size_t size)
  {
    return str;
  }

  [[nodiscard]]
  static std::pair<const char*, duk_size_t> info(const char* str)
  {
    return { str, std::strlen(str) };
  }
};


} // namespace duk


#endif // DUKCPP_STRING_TRAITS_H
