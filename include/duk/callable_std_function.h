#ifndef DUKCPP_CALLABLE_STD_FUNCTION_H
#define DUKCPP_CALLABLE_STD_FUNCTION_H

#include <duk/callable.h>
#include <functional>


namespace duk
{


template<typename T>
struct callable_traits<std::function<T>>
{
  using type = T;
};


} // namespace duk


#endif // DUKCPP_CALLABLE_STD_FUNCTION_H
