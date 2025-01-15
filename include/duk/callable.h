#ifndef DUKCPP_CALLABLE_H
#define DUKCPP_CALLABLE_H


namespace duk
{


// Used for marking function types to be treated as Function in ES
// (in contrast to being treated as Object by default).
template<typename T>
struct callable_traits_type;


// callable

template<typename T>
concept callable = requires
{
  typename callable_traits_type<T>::type;
};


template<typename T>
using callable_traits_type_t = callable_traits_type<T>::type;


// callable_traits_signature_pack

template<typename T>
struct callable_traits_signature_pack;


template<typename T>
concept has_callable_traits_signature_pack = requires
{
  typename callable_traits_signature_pack<T>::type;
};


template<typename T>
using callable_traits_signature_pack_t = callable_traits_signature_pack<T>::type;


// as_function

template<typename T>
struct as_function;


template<typename T>
struct callable_traits_type<as_function<T>>
{
  using type = T;
};


} // namespace duk


#endif // DUKCPP_CALLABLE_H
