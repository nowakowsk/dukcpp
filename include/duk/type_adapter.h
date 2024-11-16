#ifndef DUKCPP_TYPE_ADAPTER_H
#define DUKCPP_TYPE_ADAPTER_H

#include <type_traits>


namespace duk
{


template<typename T>
struct type_adapter;


template<typename T>
concept has_type_adapter = requires
{
  typename type_adapter<T>::type;
};


// type_adapter_type

template<typename T>
struct type_adapter_type
{
  using type = T; // TODO: See if this can be removed.
};

template<has_type_adapter T>
struct type_adapter_type<T>
{
  using type = type_adapter<T>::type;
};

template<typename T>
using type_adapter_type_t = typename type_adapter_type<T>::type;


// type_adapter_base

template<typename T>
concept has_type_adapter_base = requires
{
  typename type_adapter<T>::base;
  requires std::is_convertible_v<T, typename type_adapter<T>::base>;
};


template<typename T>
struct type_adapter_base;

template<has_type_adapter_base T>
struct type_adapter_base<T>
{
  using type = type_adapter<T>::base;
};

template<typename T>
using type_adapter_base_t = typename type_adapter_base<T>::type;


} // namespace duk


#endif // DUKCPP_TYPE_ADAPTER_H
