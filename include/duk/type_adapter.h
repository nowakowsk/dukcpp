#ifndef DUKCPP_TYPE_ADAPTER_H
#define DUKCPP_TYPE_ADAPTER_H


namespace duk
{


template<typename T>
struct type_adapter;


template<typename T>
concept has_type_adapter = requires
{
  typename type_adapter<T>::type;
};


template<typename T>
struct type_adapter_type
{
  using type = T;
};


template<typename T> requires has_type_adapter<T>
struct type_adapter_type<T>
{
  using type = type_adapter<T>::type;
};


template<typename T>
using type_adapter_type_t = typename type_adapter_type<T>::type;


} // namespace duk


#endif // DUKCPP_TYPE_ADAPTER_H
