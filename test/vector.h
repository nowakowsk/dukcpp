#ifndef DUKCPP_TEST_VECTOR_H
#define DUKCPP_TEST_VECTOR_H

#include "common.h"
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

  Vector operator-(const Vector& rhs);
  friend Vector operator-(const Vector& lhs, float rhs);

  float x = 0;
  float y = 0;
};


// Returns prototype object handle
void* registerVector(duk_context* ctx, duk_idx_t idx);


#endif // DUKCPP_TEST_VECTOR_H
