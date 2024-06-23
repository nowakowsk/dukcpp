#include "common.h"
#include <cmath>
#include <duk/duk.h>


Vector::Vector()
{
}


Vector::Vector(float x, float y) :
  x(x),
  y(y)
{
}


Vector::Vector(const Vector& other) :
  x(other.x),
  y(other.y)
{
}


Vector::Vector(Vector&& other) :
  x(other.x),
  y(other.y)
{
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


namespace duk
{


void* class_traits<Vector>::prototype_heapptr(duk_context* ctx)
{
  return prototype;
}


void* class_traits<Vector>::prototype = nullptr;


} // namespace duk


void* registerVector(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<Vector>,
    duk::ctor<Vector, int, int>
  >(ctx);

  duk_push_object(ctx);

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::put_function<
    static_cast<void(Vector::*)(float)>(&Vector::add),
    static_cast<void(Vector::*)(const Vector&)>(&Vector::add)
  >(ctx, -1, "add");

  duk::put_function<
    &Vector::length
  >(ctx, -1, "length");

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "Vector");

  duk::class_traits<Vector>::prototype = prototypeHandle;

  return prototypeHandle;
}


int add(int a, int b)
{
  return a + b;
}


std::string add(std::string_view a, std::string_view b)
{
  return std::string{a} + std::string{b};
}
