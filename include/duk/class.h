#ifndef DUKCPP_CLASS_H
#define DUKCPP_CLASS_H

#include <duktape.h>
#include <utility>


namespace duk
{


// class_traits

template<typename T>
struct class_traits;


template<typename T>
concept has_class_traits_prototype = requires(duk_context* ctx)
{
  { class_traits<T>::prototype_heap_ptr(ctx) } -> std::same_as<void*>;
};


// class_traits_base

template<typename T>
concept has_class_traits_base = requires
{
  typename class_traits<T>::base;
  requires std::is_base_of_v<typename class_traits<T>::base, T>;
};


template<typename T>
struct class_traits_base
{
  using type = void; // TODO: See if this can be removed.
};


template<typename T>
requires has_class_traits_base<T>
struct class_traits_base<T>
{
  using type = class_traits<T>::base;
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
