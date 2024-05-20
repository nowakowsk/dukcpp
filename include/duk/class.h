#ifndef DUKCPP_CLASS_H
#define DUKCPP_CLASS_H

#include <utility>


namespace duk
{


template<typename T>
[[nodiscard]]
T ctor(auto&& ...args)
{
  return T(std::forward<decltype(args)>(args)...);
}


} // namespace duk


#endif // DUKCPP_CLASS_H
