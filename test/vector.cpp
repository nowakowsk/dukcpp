#include "vector.h"
#include <duk/duk.h>
#include <cmath>


Vector::Vector(float x, float y) :
  x(x),
  y(y)
{
}


bool Vector::operator==(const Vector& other) const
{
  return x == other.x &&
         y == other.y;
}


float Vector::length() const
{
  return std::sqrt(x * x + y * y);
}


void Vector::add(float v)
{
  x += v;
  y += v;
}


void Vector::add(const Vector& v)
{
  x += v.x;
  y += v.y;
}


Vector operator+(const Vector& lhs, const Vector& rhs)
{
  return { lhs.x + rhs.x, lhs.y + rhs.y };
}


Vector Vector::operator-(const Vector& other)
{
  return { x - other.x, y - other.y };
}


Vector operator-(const Vector& lhs, float rhs)
{
  return { lhs.x - rhs, lhs.y - rhs };
}


void* registerVector(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<Vector>,
    duk::ctor<Vector, float, float>
  >(ctx);

  duk_push_object(ctx); // prototype

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::put_prop_function<
    static_cast<void(Vector::*)(float)>(&Vector::add),
    static_cast<void(Vector::*)(const Vector&)>(&Vector::add)
  >(ctx, -1, "add");

  duk::put_prop_method<
    &Vector::operator-,
    static_cast<Vector(*)(const Vector&, float)>(&operator-)
  >(ctx, -1, "sub");

  duk::def_prop<&Vector::x>(ctx, -1, "x");
  duk::def_prop<&Vector::y>(ctx, -1, "y");

  duk::put_prop_function<
    &Vector::length
  >(ctx, -1, "length");

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "Vector");

  duk::put_prop_function<
    static_cast<Vector(*)(const Vector&, const Vector&)>(&operator+)
  >(ctx, idx, "addVector");

  duk::class_traits_prototype<Vector>::heap_ptr = prototypeHandle;

  return prototypeHandle;
}
