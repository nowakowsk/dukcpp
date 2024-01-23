#include "lifetime.h"
#include <duk/duk.h>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>


struct DukCppTest
{
  using Allocator = duk::allocator_adapter<>;

  static void errorHandler(void* udata, const char* message)
  {
    throw std::runtime_error(message);
  }

  DukCppTest() :
    ctx_(duk_create_heap(Allocator::alloc, Allocator::realloc, Allocator::free, &allocator_, errorHandler))
  {
  }

  Allocator allocator_;
  duk::context ctx_;
};


TEST_CASE_METHOD(DukCppTest, "Register function (value arguments)")
{
  static constexpr auto addFunc = [](int a, int b)
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::register_function<addFunc>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register function (reference arguments)")
{
  static constexpr auto addFunc = [](const int& a, const int& b)
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::register_function<addFunc>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register functor")
{
  Lifetime::Observer observer;

  duk_push_global_object(ctx_);
  duk::register_function(ctx_, -1, "func", Lifetime{observer});
  duk_pop(ctx_);

  duk_eval_string(ctx_, "func()");
  ctx_.release();

  REQUIRE(observer.countersWithinLimits({
    .ctorCount = 1,
    .dtorCount = 2,
    .copyCtorCount = 0,
    .moveCtorCount = 1,
    .copyAssignmentCount = 0,
    .moveAssignmentCount = 0
  }));

  REQUIRE(observer.ctorDtorCountMatch());
}


TEST_CASE_METHOD(DukCppTest, "Register lambda with capture list")
{
  auto multiply = [a = 5](int b) -> int
  {
    return a * b;
  };

  duk_push_global_object(ctx_);
  duk::register_function(ctx_, -1, "multiply", multiply);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "multiply(10)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 50);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (non-null return value)")
{
  duk_eval_string(ctx_, "function f(a, b) { return a * b; }; (f);");
  auto f = duk::pull<std::function<int(int, int)>>(ctx_, -1);
  auto result = f(2, 3);
  REQUIRE(result == 6);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (null return value)")
{
  duk_eval_string(ctx_, "function f() {}; (f);");
  auto f = duk::pull<std::function<void()>>(ctx_, -1);
  f();
}


TEST_CASE_METHOD(DukCppTest, "Register function (function argument)")
{
  auto multiply = [](int a, std::function<int()> f) -> int
  {
    return a * f();
  };

  duk_push_global_object(ctx_);
  duk::register_function(ctx_, -1, "multiply", multiply);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    function f() { return 3; }
    multiply(10, f);
  )__");
  REQUIRE(duk::pull<int>(ctx_, -1) == 30);
}


TEST_CASE_METHOD(DukCppTest, "Allocator")
{
  using string = std::basic_string<char, std::char_traits<char>, duk::allocator<char>>;

  auto s1 = string{"s1", duk::allocator<char>(ctx_)};
  auto s2 = string{"s2", duk::allocator<char>(ctx_)};

  REQUIRE(s1 + s2 == "s1s2");
}
