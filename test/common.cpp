#include "common.h"


int add(int a, int b)
{
  return a + b;
}


std::string add(std::string_view a, std::string_view b)
{
  return std::string{a} + std::string{b};
}
