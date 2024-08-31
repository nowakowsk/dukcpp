#ifndef DUKCPP_CLASS_H
#define DUKCPP_CLASS_H

#include <duktape.h>
#include <utility>


namespace duk
{


template<typename T>
struct class_traits;


template<typename T>
concept has_class_traits_prototype = requires(duk_context* ctx)
{
  { class_traits<T>::prototype_heapptr(ctx) } -> std::same_as<void*>;
};


template<typename T>
concept has_class_traits_base = requires
{
  typename class_traits<T>::base;
};


template<typename T>
[[nodiscard]]
T ctor(auto&& ...args)
{
  return T(std::forward<decltype(args)>(args)...);
}


} // namespace duk


#endif // DUKCPP_CLASS_H
