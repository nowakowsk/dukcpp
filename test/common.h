#ifndef DUKCPP_TEST_COMMON_H
#define DUKCPP_TEST_COMMON_H

#include <duk/class.h>
#include <string>
#include <string_view>


struct Vector
{
  Vector();
  Vector(float x, float y);

  Vector(const Vector& other);
  Vector(Vector&& other);

  float length() const;

  void add(float v);
  void add(const Vector& v);

  float x = 0;
  float y = 0;
};


namespace duk
{

template<>
struct class_traits<Vector>
{
  static void* prototype_heapptr(duk_context* ctx);

  static void* prototype;
};


} // namespace duk


// Returns prototype object handle
void* registerVector(duk_context* ctx, duk_idx_t idx);


template<typename R, typename T>
R identity(T x)
{
  return x;
}


int add(int a, int b);

std::string add(std::string_view a, std::string_view b);


#endif // DUKCPP_TEST_COMMON_H