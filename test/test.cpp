#include "lifetime.h"
#include <duk/duk.h>
#include <duk/callable_std_function.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>
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
  duk::put_function<addFunc>(ctx_, -1, "addFunc");
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
  duk::put_function<addFunc>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register functor")
{
  Lifetime::Observer observer;

  duk_push_global_object(ctx_);
  duk::put_function(ctx_, -1, "func", Lifetime{observer});
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
  duk::put_function(ctx_, -1, "multiply", multiply);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "multiply(10)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 50);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (non-null return value)")
{
  duk_eval_string(ctx_, "function f(a, b) { return a * b; }; (f);");
  auto f = duk::safe_pull<std::function<int(int, int)>>(ctx_, -1);
  auto result = f(2, 3);
  REQUIRE(result == 6);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (null return value)")
{
  duk_eval_string(ctx_, "function f() {}; (f);");
  auto f = duk::safe_pull<std::function<void()>>(ctx_, -1);
  f();
}


TEST_CASE_METHOD(DukCppTest, "Register function (function argument)")
{
  auto multiply = [](int a, std::function<int()> f) -> int
  {
    return a * f();
  };

  duk_push_global_object(ctx_);
  duk::put_function(ctx_, -1, "multiply", multiply);
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


TEST_CASE_METHOD(DukCppTest, "Safe handle (function)")
{
  duk_eval_string(ctx_, "function f() { return 123; }; (f);");

  auto funcHandle = duk::safe_handle(duk::handle(ctx_, -1));

  duk_pop(ctx_); // Pop function.

  // Make sure function is available for reclamation by gc.
  // https://duktape.org/guide#limitations.12
  duk_eval_string(ctx_, "f.prototype = null; f = null;");

  duk_gc(ctx_, 0);

  auto func = duk::function<int()>(funcHandle.get());

  REQUIRE(func() == 123);
}


TEST_CASE_METHOD(DukCppTest, "Push and pull std::string")
{
  duk::push<std::string>(ctx_, "test string");
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "test string");
}


TEST_CASE_METHOD(DukCppTest, "Push and pull std::string_view")
{
  duk::push<std::string_view>(ctx_, "test string");
  auto result = (duk::pull<std::string_view>(ctx_, -1) == "test string");
  REQUIRE(result); // Needed because of some stringification issues in Catch2. Not sure what is going on here.
}


TEST_CASE_METHOD(DukCppTest, "Push and pull const char*")
{
  duk::push<const char*>(ctx_, "test string");
  REQUIRE(std::strcmp(duk::pull<const char*>(ctx_, -1), "test string") == 0);
}


// TODO: Temporary test. Replace it with something better.
TEST_CASE_METHOD(DukCppTest, "Generic object support")
{
  struct A
  {
    std::string m = "hello";
  };

  struct B
  {
    std::string m = "world";
  };

  auto f = [](const A& a, const B& b)
  {
    return a.m + b.m;
  };

  // valid arguments
  duk::push_function<f>(ctx_);
  duk::push(ctx_, A{});
  duk::push(ctx_, B{});
  duk_call(ctx_, 2);
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "helloworld");
  duk_pop(ctx_);

  // invalid arguments
  duk::push_function<f>(ctx_);
  duk::push(ctx_, A{});
  duk::push(ctx_, 1);
  REQUIRE_THROWS(duk_call(ctx_, 2));
  duk_pop(ctx_);
}
