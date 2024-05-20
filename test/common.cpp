#include "common.h"
#include <cmath>


Vector::Vector() :
  Vector(0, 0)
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


void Vector::add(float x)
{
  this->x += x;
  this->y += x;
}


void Vector::add(const Vector& v)
{
  x += v.x;
  y += v.y;
}


int add(int a, int b)
{
  return a + b;
}


std::string add(std::string_view a, std::string_view b)
{
  return std::string{a} + std::string{b};
}
