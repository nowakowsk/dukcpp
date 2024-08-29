#include "lifetime.h"
#include <duk/duk.h>
#include <catch2/catch_test_macros.hpp>
#include <string>


using duk::detail::any;
using duk::detail::bad_any_cast;


TEST_CASE("any (int)")
{
  any sa(100);
  REQUIRE(sa.cast<int>() == 100);
}


TEST_CASE("any (modify by reference)")
{
  any sa(100);
  sa.cast<int&>() = 101;
  REQUIRE(sa.cast<int>() == 101);
}


TEST_CASE("any (std::string)")
{
  any sa(std::string("test"));
  REQUIRE(sa.cast<std::string>() == "test");
}


TEST_CASE("any (Lifetime)")
{
  Lifetime::Observer observer;
  {
    any sa{Lifetime(observer)};
  }
  REQUIRE(observer.countersWithinLimits({
    .ctorCount = 1,
    .dtorCount = 2,
    .copyCtorCount = 0,
    .moveCtorCount = 1,
    .copyAssignmentCount = 0,
    .moveAssignmentCount = 0
  }));
}


TEST_CASE("any (bad cast)")
{
  any sa(100);
  REQUIRE_THROWS_AS(sa.cast<float>(), bad_any_cast);
}


TEST_CASE("any (const 1)")
{
  any sa(static_cast<const int>(100));
  REQUIRE(sa.cast<int>() == 100);
  REQUIRE(sa.cast<const int>() == 100);
  REQUIRE(sa.cast<int&>() == 100);
  REQUIRE(sa.cast<const int&>() == 100);
}


TEST_CASE("any (const 2)")
{
  const any sa(static_cast<const int>(100));
  REQUIRE(sa.cast<int>() == 100);
  REQUIRE(sa.cast<const int>() == 100);
  REQUIRE(sa.cast<const int&>() == 100);
}


TEST_CASE("any (const 3)")
{
  const any sa(100);
  REQUIRE(sa.cast<int>() == 100);
  REQUIRE(sa.cast<const int>() == 100);
  REQUIRE(sa.cast<const int&>() == 100);
}
