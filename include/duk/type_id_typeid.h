#ifndef DUKCPP_TYPE_ID_TYPEID_H
#define DUKCPP_TYPE_ID_TYPEID_H

#include <typeindex>


namespace duk
{


template<typename T>
[[nodiscard]]
constexpr std::size_t type_id() noexcept
{
  return typeid(T).hash_code();
}


} // namespace duk


#endif // DUKCPP_TYPE_ID_TYPEID_H
