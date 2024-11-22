#ifndef DUKCPP_TEST_VECTOR_H
#define DUKCPP_TEST_VECTOR_H

#include <duk/class.h>


struct Vector
{
  Vector() = default;
  Vector(float x, float y);

  bool operator==(const Vector& other) const;

  [[nodiscard]]
  float length() const;

  void add(float v);
  void add(const Vector& v);

  friend Vector operator+(const Vector& lhs, const Vector& rhs);

  float x = 0;
  float y = 0;
};


namespace duk
{


template<>
struct class_traits<Vector>
{
  static void* prototype_heap_ptr(duk_context* ctx);

  static void* prototype;
};


} // namespace duk


// Returns prototype object handle
void* registerVector(duk_context* ctx, duk_idx_t idx);


#endif // DUKCPP_TEST_VECTOR_H
