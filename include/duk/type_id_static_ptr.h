#ifndef DUKCPP_TYPE_ID_STATIC_PTR_H
#define DUKCPP_TYPE_ID_STATIC_PTR_H

#include <cstddef>
#include <cstdint>


namespace duk
{


template<typename T>
[[nodiscard]]
constexpr std::size_t type_id() noexcept
{
  constexpr static char dummy = 0;

  return static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(&dummy));
}


} // namespace duk


#endif // DUKCPP_TYPE_ID_STATIC_PTR_H
