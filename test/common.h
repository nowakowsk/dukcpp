#ifndef DUKCPP_TEST_COMMON_H
#define DUKCPP_TEST_COMMON_H

#include <duk/class.h>
#include <duk/type_adapter.h>
#include <cmath>
#include <memory>
#include <string>
#include <string_view>


namespace duk
{


template<typename T>
struct type_adapter<std::shared_ptr<T>>
{
  using type = T;

  using base = std::conditional_t<
    has_class_traits_base<T>,
    std::shared_ptr<class_traits_base_t<T>>,
    void
  >;

  template<typename U>
  [[nodiscard]]
  static U get(std::shared_ptr<T> ptr)
  {
    return *ptr;
  }
};


template<typename T>
struct class_traits_prototype
{
  [[nodiscard]]
  static void* get(duk_context*) noexcept
  {
    return heap_ptr;
  }

  inline static void* heap_ptr = nullptr;
};


} // namespace duk


template<typename T>
[[nodiscard]]
bool equals(const T& a, const T& b, const T& e)
{
  return std::abs(a - b) <= e;
}


template<typename R, typename T>
R identity(T x)
{
  return x;
}


int add(int a, int b);

std::string add(std::string_view a, std::string_view b);


#endif // DUKCPP_TEST_COMMON_H
