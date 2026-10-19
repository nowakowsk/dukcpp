#ifndef DUKCPP_CLASS_H
#define DUKCPP_CLASS_H

#include <duk/common.h>
#include <duk/string_traits.h>
#include <duktape.h>
#include <string_view>
#include <utility>


namespace duk
{


// class_traits_name

template<typename T>
struct class_traits_name
{
  [[nodiscard]]
  static std::string_view get(duk_context*)
  {
    return "unknown";
  }
};


template<typename T>
requires(integer<T> || floating_point<T> || enumeration<T>)
struct class_traits_name<T>
{
  [[nodiscard]]
  static std::string_view get(duk_context*)
  {
    return "number";
  }
};


template<boolean T>
struct class_traits_name<T>
{
  [[nodiscard]]
  static std::string_view get(duk_context*)
  {
    return "boolean";
  }
};


template<object T>
struct class_traits_name<T>
{
  [[nodiscard]]
  static std::string_view get(duk_context*)
  {
    return "object";
  }
};


template<string_type T>
struct class_traits_name<T>
{
  [[nodiscard]]
  static std::string_view get(duk_context*)
  {
    return "string";
  }
};


// class_traits_prototype

template<typename T>
struct class_traits_prototype;


template<typename T>
concept has_class_traits_prototype = requires(duk_context* ctx)
{
  { class_traits_prototype<T>::get(ctx) } -> std::same_as<void*>;
};


// class_traits_base

template<typename T>
struct class_traits_base
{
  // void base type means that there is no base. I don't like it, but it makes things easier.
  using type = void;
};


template<typename T>
concept has_class_traits_base = requires
{
  typename class_traits_base<T>::type;
  requires std::is_base_of_v<typename class_traits_base<T>::type, T>;
};


template<typename T>
using class_traits_base_t = typename class_traits_base<T>::type;


// ctor

template<typename T>
[[nodiscard]]
T ctor(auto&& ...args)
{
  return T(std::forward<decltype(args)>(args)...);
}


} // namespace duk


#endif // DUKCPP_CLASS_H
