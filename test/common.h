#ifndef DUKCPP_TEST_COMMON_H
#define DUKCPP_TEST_COMMON_H

#include <cmath>
#include <string>
#include <string_view>


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
