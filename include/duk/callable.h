#ifndef DUKCPP_CALLABLE_H
#define DUKCPP_CALLABLE_H


namespace duk
{


// Used for marking function types to be treated as Function in ES
// (in contrast to being treated as Object by default).
template<typename T>
struct callable_traits;


template<typename T>
concept callable =
  requires(T)
  {
    typename callable_traits<T>::type;
  };


template<typename T>
struct as_function;


template<typename T>
struct callable_traits<as_function<T>>
{
  using type = T;
};


} // namespace duk


#endif // DUKCPP_CALLABLE_H
