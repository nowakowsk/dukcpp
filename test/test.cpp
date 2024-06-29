#include "common.h"
#include "lifetime.h"
#include "vector.h"
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
  static constexpr auto add = [](int a, int b) -> int
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::put_function<add>(ctx_, -1, "add");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register function (reference arguments)")
{
  static constexpr auto add = [](const int& a, const int& b)
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::put_function<add>(ctx_, -1, "add");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: int")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<int, int>>(ctx_, -1, "int_int");
  duk::put_function<identity<int, const int&>>(ctx_, -1, "int_crefint");
  duk::put_function<identity<const int&, const int&>>(ctx_, -1, "crefint_crefint");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "int_int(-5)");
  REQUIRE(duk::pull<int>(ctx_, -1) == -5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "int_crefint(-5)");
  REQUIRE(duk::pull<int>(ctx_, -1) == -5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefint_crefint(-5)");
  REQUIRE(duk::pull<const int&>(ctx_, -1) == -5);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: unsigned int")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<unsigned int, unsigned int>>(ctx_, -1, "uint_uint");
  duk::put_function<identity<unsigned int, const unsigned int&>>(ctx_, -1, "uint_crefuint");
  duk::put_function<identity<const unsigned int&, const unsigned int&>>(ctx_, -1, "crefuint_crefuint");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "uint_uint(5)");
  REQUIRE(duk::pull<unsigned int>(ctx_, -1) == 5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "uint_crefuint(5)");
  REQUIRE(duk::pull<unsigned int>(ctx_, -1) == 5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefuint_crefuint(5)");
  REQUIRE(duk::pull<const unsigned int&>(ctx_, -1) == 5);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: bool")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<bool, bool>>(ctx_, -1, "bool_bool");
  duk::put_function<identity<bool, const bool&>>(ctx_, -1, "bool_crefbool");
  duk::put_function<identity<const bool&, const bool&>>(ctx_, -1, "crefbool_crefbool");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "bool_bool(true)");
  REQUIRE(duk::pull<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "bool_crefbool(true)");
  REQUIRE(duk::pull<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefbool_crefbool(true)");
  REQUIRE(duk::pull<const bool&>(ctx_, -1) == true);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: float")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<float, float>>(ctx_, -1, "float_float");
  duk::put_function<identity<float, const float&>>(ctx_, -1, "float_creffloat");
  duk::put_function<identity<const float&, const float&>>(ctx_, -1, "creffloat_creffloat");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "float_float(1)");
  REQUIRE(equals(duk::pull<float>(ctx_, -1), 1.0f, 1e-5f));
  duk_pop(ctx_);

  duk_eval_string(ctx_, "float_creffloat(1)");
  REQUIRE(equals(duk::pull<float>(ctx_, -1), 1.0f, 1e-5f));
  duk_pop(ctx_);

  duk_eval_string(ctx_, "creffloat_creffloat(1)");
  REQUIRE(equals(duk::pull<const float&>(ctx_, -1), 1.0f, 1e-5f));
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: std::string")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<std::string, std::string>>(ctx_, -1, "string_string");
  duk::put_function<identity<std::string, const std::string&>>(ctx_, -1, "string_crefstring");
  duk::put_function<identity<const std::string&, const std::string&>>(ctx_, -1, "crefstring_crefstring");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "string_string('test')");
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "string_crefstring('test')");
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefstring_crefstring('test')");
  REQUIRE(duk::pull<const std::string&>(ctx_, -1) == "test");
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: std::string_view")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<std::string_view, std::string_view>>(ctx_, -1, "string_string");
  duk::put_function<identity<std::string_view, const std::string_view&>>(ctx_, -1, "string_crefstring");
  duk::put_function<identity<const std::string_view&, const std::string_view&>>(ctx_, -1, "crefstring_crefstring");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "(string_string('test'))");
  REQUIRE(duk::pull<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "(string_crefstring('test'))");
  REQUIRE(duk::pull<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "(crefstring_crefstring('test'))");
  REQUIRE(duk::pull<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: const char*")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<const char*, const char*>>(ctx_, -1, "constchar_constchar");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "constchar_constchar('test')");
  REQUIRE(std::strcmp(duk::pull<const char*>(ctx_, -1), "test") == 0);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Register overloaded function (value arguments)")
{
  // TODO: This shouldn't be needed, but doing those casts directly in duk::put_function call fails to compile under
  // MSVC. Bug report: https://developercommunity.visualstudio.com/t/Code-compiles-in-GCC-and-Clang-but-fail/10673264
  static constexpr auto add1 = static_cast<int(*)(int, int)>(add);
  static constexpr auto add2 = static_cast<std::string(*)(std::string_view, std::string_view)>(add);

  duk_push_global_object(ctx_);
  duk::put_function<add1, add2>(ctx_, -1, "add");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add(1, 2)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 3);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add('a', 'b')");
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "ab");
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Register constexpr functor")
{
  struct Functor
  {
    constexpr Functor()
    {
    }

    bool operator()() const
    {
      return true;
    }

    int operator()(int x) const
    {
      return x;
    }
  };

  duk_push_global_object(ctx_);
  duk::put_function<
    duk::function_descriptor<Functor{}, bool(), int(int)>
  >(ctx_, -1, "func");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "func()");
  REQUIRE(duk::pull<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "func(10)");
  REQUIRE(duk::pull<int>(ctx_, -1) == 10);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Check object copy counts")
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
  duk::push(ctx_, std::string("test string"));
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "test string");
}


TEST_CASE_METHOD(DukCppTest, "Push and pull std::string_view")
{
  duk::push(ctx_, std::string_view("test string"));
  REQUIRE(duk::pull<std::string_view>(ctx_, -1) == "test string");
}


TEST_CASE_METHOD(DukCppTest, "Push and pull const char*")
{
  duk::push(ctx_, "test string");
  REQUIRE(std::strcmp(duk::pull<const char*>(ctx_, -1), "test string") == 0);
}


TEST_CASE_METHOD(DukCppTest, "Class binding")
{
  duk_push_global_object(ctx_);

  auto prototypeHandle = registerVector(ctx_, -1);

  // Primitive constructor function
  duk::push_function<duk::ctor<Vector>>(ctx_);
  duk_put_prop_string(ctx_, -2, "makeVector");

  duk::push(ctx_, Vector{}, prototypeHandle);
  duk_put_prop_string(ctx_, -2, "v2");

  static constexpr auto addOperator = [](const Vector& lhs, const Vector& rhs) { return lhs + rhs; };
  duk::push_function<addOperator>(ctx_);
  duk_put_prop_string(ctx_, -2, "addVector");

  duk_pop(ctx_); // Pop global object

  duk_eval_string(ctx_, R"__(
    var v1 = new Vector(1, 2);
    v1.add(2);
    v1.length();
  )__");
  REQUIRE(equals(duk::pull<double>(ctx_, -1), 5.0, 1e-5));
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    v1.add(new Vector(2, 8));
    v1.length();
  )__");
  REQUIRE(equals(duk::pull<double>(ctx_, -1), 13.0, 1e-5));
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    v3 = makeVector();
    v3.length();
    v1.add(v3);
    v1 instanceof Vector && v2 instanceof Vector && v3 instanceof Vector
  )__");
  REQUIRE(duk::pull<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    addVector(new Vector(1, 1), new Vector(2, 3)).length()
  )__");
  REQUIRE(equals(duk::pull<float>(ctx_, -1), 5.0f, 1e-5f));
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Generic object binding")
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

  // Valid arguments
  duk::push_function<f>(ctx_);
  duk::push(ctx_, A{});
  duk::push(ctx_, B{});
  duk_call(ctx_, 2);
  REQUIRE(duk::pull<std::string>(ctx_, -1) == "helloworld");
  duk_pop(ctx_);

  // Invalid arguments
  duk::push_function<f>(ctx_);
  duk::push(ctx_, A{});
  duk::push(ctx_, 1);
  REQUIRE_THROWS(duk_call(ctx_, 2));
  duk_pop(ctx_);
}
