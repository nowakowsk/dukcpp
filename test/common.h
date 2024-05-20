#ifndef DUKCPP_TEST_COMMON_H
#define DUKCPP_TEST_COMMON_H

#include <string>
#include <string_view>


struct Vector
{
  Vector();
  Vector(float x, float y);

  Vector(const Vector& other);
  Vector(Vector&& other);

  float length() const;

  void add(float x);
  void add(const Vector& v);

  float x;
  float y;
};


template<typename R, typename T>
R identity(T x)
{
  return x;
}


int add(int a, int b);

std::string add(std::string_view a, std::string_view b);


#endif // DUKCPP_TEST_COMMON_H