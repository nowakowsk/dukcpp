#ifndef DUKCPP_CALLABLE_H
#define DUKCPP_CALLABLE_H


namespace duk
{


// Used for marking function types to be treated as Function in ES
// (in contrast to being treated as Object by default).
template<typename T>
struct callable_traits;


// callable

template<typename T>
concept callable = requires
{
  typename callable_traits<T>::type;
};

template<typename T>
using callable_traits_t = callable_traits<T>::type;


// callable_traits_signature_pack

template<typename T>
concept has_callable_traits_signature_pack = requires
{
  typename callable_traits<T>::signature_pack;
};

template<typename T>
using callable_traits_signature_pack = callable_traits<T>::signature_pack;


// as_function

template<typename T>
struct as_function;


template<typename T>
struct callable_traits<as_function<T>>
{
  using type = T;
};


} // namespace duk


#endif // DUKCPP_CALLABLE_H
