#ifndef DUKCPP_CLASS_H
#define DUKCPP_CLASS_H

#include <duk/common.h>
#include <duk/detail/std.h>
#include <duk/iterable.h>
#include <duk/string_traits.h>
#include <duktape.h>
#include <string_view>
#include <utility>


namespace duk
{


// class_traits_name

template<typename T>
struct class_traits_name;


template<typename T>
concept has_class_traits_name = requires(duk_context* ctx)
{
  { class_traits_name<T>::get(ctx) } -> std::same_as<std::string_view>;
};


namespace detail
{


template<typename T>
string classTraitsName(duk_context* ctx)
{
  if constexpr (has_class_traits_name<T>)
  {
    if (auto name = class_traits_name<T>::get(ctx); !name.empty())
      return make_string(ctx, name);
  }

  if constexpr (boolean<T>)
    return make_string(ctx, "Boolean");

  if constexpr (integer<T> || floating_point<T> || enumeration<T>)
    return make_string(ctx, "Number");

  if constexpr (string_type<T>)
    return make_string(ctx, "String");

  if constexpr (callable<T>)
    return make_string(ctx, "Function");

  if constexpr (iterable<T>)
  {
    using IterT = std::decay_t<decltype(*begin(T{}))>;

    return "Iterable[" + classTraitsName<IterT>(ctx) + "]";
  }

  if constexpr (object<T>)
    return make_string(ctx, "Object");

  return make_string(ctx, "[unknown]");
}


} // namespace detail


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
