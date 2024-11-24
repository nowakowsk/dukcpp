#include "character.h"
#include "common.h"
#include "functor.h"
#include "inheritance.h"
#include "lifetime.h"
#include "vector.h"
#include <duk/duk.h>
#include <duk/callable_std_function.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <cstring>
#include <numeric>
#include <stdexcept>
#include <string>


// Check if iterators and ranges conform to standard requirements of their categories.
//
// NOTE:
// I don't think duk::array_input_iterator<T> really meets requirements of std::random_access_iterator, because
// return type of its operator[] isn't always convertible to a reference (this depends on T).
static_assert(std::random_access_iterator<duk::array_input_iterator<int>>);
static_assert(std::ranges::random_access_range<duk::array_input_range<int>>);

static_assert(std::input_iterator<duk::symbol_input_iterator<int>>);
static_assert(std::ranges::input_range<duk::symbol_input_range<int>>);

static_assert(std::input_iterator<duk::input_iterator<int>>);
static_assert(std::ranges::input_range<duk::input_range<int>>);


// Check if handles match duk::handle_type concept.

static_assert(duk::handle_type<duk::handle>);
static_assert(duk::handle_type<duk::safe_handle>);


// Check if strings match duk::string_type concept.
static_assert(duk::string_type<const char*>);
static_assert(duk::string_type<std::string>);
static_assert(duk::string_type<std::string_view>);


struct DukCppTest
{
  using Allocator = duk::allocator_adapter<>;

  static void errorHandler([[maybe_unused]] void* udata, const char* message)
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


template<typename T>
struct DukCppTemplateTest : public DukCppTest
{
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
  REQUIRE(duk::get<int>(ctx_, -1) == 3);
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
  REQUIRE(duk::get<int>(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: int")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<int, int>>(ctx_, -1, "int_int");
  duk::put_function<identity<int, const int&>>(ctx_, -1, "int_crefint");
  duk::put_function<identity<const int&, const int&>>(ctx_, -1, "crefint_crefint");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "int_int(-5)");
  REQUIRE(duk::get<int>(ctx_, -1) == -5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "int_crefint(-5)");
  REQUIRE(duk::get<int>(ctx_, -1) == -5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefint_crefint(-5)");
  REQUIRE(duk::get<const int&>(ctx_, -1) == -5);
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
  REQUIRE(duk::get<unsigned int>(ctx_, -1) == 5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "uint_crefuint(5)");
  REQUIRE(duk::get<unsigned int>(ctx_, -1) == 5);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefuint_crefuint(5)");
  REQUIRE(duk::get<const unsigned int&>(ctx_, -1) == 5);
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
  REQUIRE(equals(duk::get<float>(ctx_, -1), 1.0f, 1e-5f));
  duk_pop(ctx_);

  duk_eval_string(ctx_, "float_creffloat(1)");
  REQUIRE(equals(duk::get<float>(ctx_, -1), 1.0f, 1e-5f));
  duk_pop(ctx_);

  duk_eval_string(ctx_, "creffloat_creffloat(1)");
  REQUIRE(equals(duk::get<const float&>(ctx_, -1), 1.0f, 1e-5f));
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
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "bool_crefbool(true)");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefbool_crefbool(true)");
  REQUIRE(duk::get<const bool&>(ctx_, -1) == true);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: enum")
{
  enum class Enum
  {
    A, B, C
  };

  duk_push_global_object(ctx_);
  duk::put_function<identity<Enum, Enum>>(ctx_, -1, "enum_enum");
  duk::put_function<identity<Enum, const Enum&>>(ctx_, -1, "enum_crefenum");
  duk::put_function<identity<const Enum&, const Enum&>>(ctx_, -1, "crefenum_crefenum");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "enum_enum(0)");
  REQUIRE(duk::get<Enum>(ctx_, -1) == Enum::A);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "enum_crefenum(1)");
  REQUIRE(duk::get<Enum>(ctx_, -1) == Enum::B);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefenum_crefenum(2)");
  REQUIRE(duk::get<const Enum&>(ctx_, -1) == Enum::C);
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
  REQUIRE(duk::get<std::string>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "string_crefstring('test')");
  REQUIRE(duk::get<std::string>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "crefstring_crefstring('test')");
  REQUIRE(duk::get<const std::string&>(ctx_, -1) == "test");
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
  REQUIRE(duk::get<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "(string_crefstring('test'))");
  REQUIRE(duk::get<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "(crefstring_crefstring('test'))");
  REQUIRE(duk::get<std::string_view>(ctx_, -1) == "test");
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Pass and return by value and const reference: const char*")
{
  duk_push_global_object(ctx_);
  duk::put_function<identity<const char*, const char*>>(ctx_, -1, "constchar_constchar");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "constchar_constchar('test')");
  REQUIRE(std::strcmp(duk::get<const char*>(ctx_, -1), "test") == 0);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Register overloaded function (value arguments)")
{
  duk_push_global_object(ctx_);
  duk::put_function<
    static_cast<int(*)(int, int)>(add),
    static_cast<std::string(*)(std::string_view, std::string_view)>(add)
  >(ctx_, -1, "add");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add(1, 2)");
  REQUIRE(duk::get<int>(ctx_, -1) == 3);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "add('a', 'b')");
  REQUIRE(duk::get<std::string>(ctx_, -1) == "ab");
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Register functor (with duk::callable_traits)")
{
  duk_push_global_object(ctx_);

  duk::put_function<makeFunctor>(ctx_, -1, "makeFunctor");

  SECTION("constexpr")
  {
    duk::put_function<
      duk::function_descriptor<Functor{}, bool(), int(int)>
    >(ctx_, -1, "func");
  }

  SECTION("non-constexpr")
  {
    Functor f;

    duk::put_function<bool(), int(int)>(ctx_, -1, "func", f);
  }

  duk_pop(ctx_); // duk_push_global_object

  duk_eval_string(ctx_, "func()");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "func(10)");
  REQUIRE(duk::get<int>(ctx_, -1) == 10);
  duk_pop(ctx_);

  // Functor returned from a function is treated as a Function because it specializes duk::callable_traits.
  duk_eval_string(ctx_, "var f = makeFunctor();");

  duk_eval_string(ctx_, "f instanceof Function");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "f()");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "f(20)");
  REQUIRE(duk::get<int>(ctx_, -1) == 20);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Register functor (without duk::callable_traits)")
{
  // This class shadows Functor defined globally in functor.h.
  struct Functor
  {
    constexpr Functor() = default;

    bool operator()() const
    {
      return true;
    }

    int operator()(int x) const
    {
      return x;
    }
  };

  static constexpr auto makeFunctor = []() { return Functor{}; };

  duk_push_global_object(ctx_);

  duk::put_function<makeFunctor>(ctx_, -1, "makeFunctor");

  SECTION("constexpr")
  {
    duk::put_function<
      duk::function_descriptor<Functor{}, bool(), int(int)>
    >(ctx_, -1, "func");
  }

  SECTION("non-constexpr")
  {
    Functor f;

    duk::put_function<bool(), int(int)>(ctx_, -1, "func", f);
  }

  duk_pop(ctx_); // duk_push_global_object

  duk_eval_string(ctx_, "func()");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "func(10)");
  REQUIRE(duk::get<int>(ctx_, -1) == 10);
  duk_pop(ctx_);

  // Functor returned from a function is treated as an Object because it doesn't specialize duk::callable_traits.
  duk_eval_string(ctx_, "var f = makeFunctor();");

  duk_eval_string(ctx_, "f instanceof Object");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "f instanceof Function");
  REQUIRE(duk::get<bool>(ctx_, -1) == false);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Check object copy counts")
{
  Lifetime::Observer observer;

  duk_push_global_object(ctx_);

  duk::put_prop(ctx_, -1, "func", Lifetime{observer});

  duk_pop(ctx_); // duk_push_global_object

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
  REQUIRE(duk::get<int>(ctx_, -1) == 50);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (non-null return value)")
{
  duk_eval_string(ctx_, "function f(a, b) { return a * b; }; (f);");
  auto f = duk::get<std::function<int(int, int)>>(ctx_, -1);
  auto result = f(2, 3);
  REQUIRE(result == 6);
}


TEST_CASE_METHOD(DukCppTest, "Call ES function in C++ (null return value)")
{
  duk_eval_string(ctx_, "function f() {}; (f);");
  auto f = duk::get<std::function<void()>>(ctx_, -1);
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
  REQUIRE(duk::get<int>(ctx_, -1) == 30);
}


TEST_CASE_METHOD(DukCppTest, "handle")
{
  duk::scoped_pop _(ctx_); // duk::push
  duk::push<std::string>(ctx_, {});
  auto heapPtr = duk_get_heapptr(ctx_, -1);

  duk::handle h1;
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);

  h1 = duk::handle(ctx_, heapPtr);
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);

  duk::handle h2(h1);
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);
  REQUIRE(h2.ctx() == ctx_);
  REQUIRE(h2.heap_ptr() == heapPtr);

  duk::handle h3(std::move(h1));
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);

  h1 = h3;
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);

  h3 = std::move(h1);
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);
}


TEST_CASE_METHOD(DukCppTest, "safe_handle")
{
  const auto makeObject = [&](int id)
  {
    duk::scoped_pop _(ctx_); // duk_push_object
    duk_push_object(ctx_);

    duk_push_int(ctx_, id);
    duk_put_prop_index(ctx_, -2, 0);

    return duk::safe_handle(duk::handle(ctx_, -1));
  };

  static constexpr auto getObjectId = [](const duk::safe_handle& handle)
  {
    auto ctx = handle.ctx();

    duk::scoped_pop _(ctx); // push_handle
    push_handle(handle);

    duk::scoped_pop __(ctx); // duk_get_prop_index
    duk_get_prop_index(ctx, -1, 0);

    return duk_get_int(ctx, -1);
  };

  auto h0 = makeObject(0);

  duk::safe_handle h1;
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);

  h1 = makeObject(1);
  auto heapPtr = h1.heap_ptr();
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);

  duk::safe_handle h2(h1);
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);
  REQUIRE(h2.ctx() == ctx_);
  REQUIRE(h2.heap_ptr() == heapPtr);

  duk::safe_handle h3(std::move(h1));
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);

  h1 = h3;
  REQUIRE(h1.ctx() == ctx_);
  REQUIRE(h1.heap_ptr() == heapPtr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);

  h3 = std::move(h1);
  REQUIRE(h1.ctx() == nullptr);
  REQUIRE(h1.heap_ptr() == nullptr);
  REQUIRE(h3.ctx() == ctx_);
  REQUIRE(h3.heap_ptr() == heapPtr);

  duk_gc(ctx_, 0);

  REQUIRE(getObjectId(h0) == 0);
  REQUIRE(getObjectId(h3) == 1);
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

  auto func = duk::safe_function_handle<int()>(funcHandle);

  REQUIRE(func() == 123);
}


TEST_CASE_METHOD(DukCppTest, "Push and get std::string")
{
  duk::push(ctx_, std::string("test string"));
  REQUIRE(duk::get<std::string>(ctx_, -1) == "test string");
}


TEST_CASE_METHOD(DukCppTest, "Push and get std::string_view")
{
  duk::push(ctx_, std::string_view("test string"));
  REQUIRE(duk::get<std::string_view>(ctx_, -1) == "test string");
}


TEST_CASE_METHOD(DukCppTest, "Push and get const char*")
{
  duk::push(ctx_, "test string");
  REQUIRE(std::strcmp(duk::get<const char*>(ctx_, -1), "test string") == 0);
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

  duk::push(ctx_, std::make_shared<Vector>(1.0f, 1.0f), prototypeHandle);
  duk_put_prop_string(ctx_, -2, "v4");

  duk_pop(ctx_); // Pop global object

  duk_eval_string(ctx_, R"__(
    var v1 = new Vector(1, 2);
    v1.add(2);
    v1.length();
  )__");
  REQUIRE(equals(duk::get<double>(ctx_, -1), 5.0, 1e-5));
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    v1.add(new Vector(2, 8));
    v1.length();
  )__");
  REQUIRE(equals(duk::get<double>(ctx_, -1), 13.0, 1e-5));
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    v3 = makeVector();
    v3.length();
    v1.add(v3);
    v1 instanceof Vector && v2 instanceof Vector && v3 instanceof Vector
  )__");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    addVector(v4, new Vector(2, 3)).length()
  )__");
  REQUIRE(equals(duk::get<float>(ctx_, -1), 5.0f, 1e-5f));
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    v4.x = 100;
    v4.x;
  )__");
  REQUIRE(equals(duk::get<double>(ctx_, -1), 100.0, 1e-5));
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Inheritance")
{
  auto assertEq = [this](const char* code, const char* result)
  {
    duk_eval_string(ctx_, code);
    REQUIRE(duk::get<std::string>(ctx_, -1) == result);
    duk_pop(ctx_);
  };

  static constexpr auto runMethodA = [](InhBase& obj) { return obj.methodA(); };
  static constexpr auto runMethodB = [](InhDer& obj) { return obj.methodB(); };
  static constexpr auto runMethodC = [](InhFinal& obj) { return obj.methodC(); };

  static constexpr auto runMethodPtrA = [](std::shared_ptr<InhBase> obj) { return obj->methodA(); };
  static constexpr auto runMethodPtrB = [](std::shared_ptr<InhDer> obj) { return obj->methodB(); };
  static constexpr auto runMethodPtrC = [](std::shared_ptr<InhFinal> obj) { return obj->methodC(); };

  duk_push_global_object(ctx_);

  // These need to be called in that order to make sure base class prototypes are initialized.
  registerInhBase(ctx_, -1);
  registerInhDer(ctx_, -1);
  registerInhFinal(ctx_, -1);

  duk::put_function<std::make_shared<InhBase>>(ctx_, -1, "makeInhBase");
  duk::put_function<std::make_shared<InhDer>>(ctx_, -1, "makeInhDer");
  duk::put_function<std::make_shared<InhFinal>>(ctx_, -1, "makeInhFinal");

  duk::put_function<runMethodA>(ctx_, -1, "runMethodA");
  duk::put_function<runMethodB>(ctx_, -1, "runMethodB");
  duk::put_function<runMethodC>(ctx_, -1, "runMethodC");

  duk::put_function<runMethodPtrA>(ctx_, -1, "runMethodPtrA");
  duk::put_function<runMethodPtrB>(ctx_, -1, "runMethodPtrB");
  duk::put_function<runMethodPtrC>(ctx_, -1, "runMethodPtrC");

  duk_pop(ctx_); // Pop global object

  SECTION("Values")
  {
    duk_eval_string(ctx_, R"__(
      var base = new InhBase();
      var der = new InhDer();
      var final = new InhFinal();
    )__");
  }

  SECTION("Pointers")
  {
    duk_eval_string(ctx_, R"__(
      var base = makeInhBase();
      var der = makeInhDer();
      var final = makeInhFinal();
    )__");

    assertEq("runMethodPtrA(base);", "BaseA");
    
    assertEq("runMethodPtrA(der);", "DerA");
    assertEq("runMethodPtrB(der);", "DerB");

    assertEq("runMethodPtrA(final);", "FinalA");
    assertEq("runMethodPtrB(final);", "FinalB");
    assertEq("runMethodPtrC(final);", "FinalC");
  }

  assertEq("base.methodA();", "BaseA");

  assertEq("der.methodA();", "DerA");
  assertEq("der.methodB();", "DerB");

  assertEq("final.methodA();", "FinalA");
  assertEq("final.methodB();", "FinalB");
  assertEq("final.methodC();", "FinalC");

  assertEq("runMethodA(base);", "BaseA");
  
  assertEq("runMethodA(der);", "DerA");
  assertEq("runMethodB(der);", "DerB");

  assertEq("runMethodA(final);", "FinalA");
  assertEq("runMethodB(final);", "FinalB");
  assertEq("runMethodC(final);", "FinalC");

  REQUIRE_THROWS(duk_eval_string(ctx_, "base.methodC();"));
}


TEMPLATE_TEST_CASE_METHOD(DukCppTemplateTest, "Manual finalization", "",
  Lifetime,
  CallableLifetime
)
{
  auto& ctx = DukCppTest::ctx_;

  duk_push_global_object(ctx);

  Lifetime::Observer observer;
  duk::push(ctx, TestType{observer});

  REQUIRE(observer.ctorDtorCountMatch() == false);
  REQUIRE(duk::finalize(ctx, -1) == true);
  REQUIRE(observer.ctorDtorCountMatch() == true);

  // Double-free
  REQUIRE(duk::finalize(ctx, -1) == false);
}


TEST_CASE_METHOD(DukCppTest, "Enum")
{
  enum class Enum
  {
    E1, E2
  };

  duk_push_global_object(ctx_);

  duk::def_prop_enum(ctx_, -1, "Enum",
    Enum::E1, "E1",
    Enum::E2, "E2"
  );

  duk_pop(ctx_); // Pop global object

  duk_eval_string(ctx_, "Enum.E1");
  REQUIRE(duk::get<Enum>(ctx_, -1) == Enum::E1);
  duk_pop(ctx_);

  duk_eval_string(ctx_, "Enum.E2");
  REQUIRE(duk::get<Enum>(ctx_, -1) == Enum::E2);
  duk_pop(ctx_);
}


TEST_CASE_METHOD(DukCppTest, "Properties")
{
  duk_push_global_object(ctx_);

  registerVector(ctx_, -1);
  registerCharacter(ctx_, -1);

  duk_pop(ctx_); // Pop global object

  duk_eval_string(ctx_, R"__(
    var c = new Character();
    c.id = 100;
    c.id;
  )__");
  REQUIRE(duk::get<int>(ctx_, -1) == 100);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    c.name = "name";
    c.name;
  )__");
  REQUIRE(duk::get<std::string>(ctx_, -1) == "name");
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    c.active = true;
    c.active;
  )__");
  REQUIRE(duk::get<bool>(ctx_, -1) == true);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    c.type = Character.Type.ENEMY;
    c.type;
  )__");
  REQUIRE(duk::get<Character::Type>(ctx_, -1) == Character::Type::ENEMY);
  duk_pop(ctx_);

  duk_eval_string(ctx_, R"__(
    c.position = new Vector(10, 20);
    c.position;
  )__");
  REQUIRE(duk::get<Vector>(ctx_, -1) == Vector(10, 20));
  duk_pop(ctx_);
}


TEMPLATE_TEST_CASE_METHOD(DukCppTemplateTest, "Ranges (std::input_iterator)", "",
  duk::safe_array_input_range<int>,
  duk::safe_symbol_input_range<char>,
  duk::safe_input_range<int>,
  duk::safe_input_range<char>
)
{
  auto& ctx = DukCppTest::ctx_;

  const char* containerCode = nullptr;
  if constexpr (std::is_same_v<decltype(*std::declval<TestType>().begin()), int>)
  {
    containerCode = "([0, 1, 2, 3, 4]);";
  }
  else
  {
    containerCode = R"__(
      var iterableObject = {
        [Symbol.iterator]: function()
        {
          var n = -1;
          var done = false;

          return {
            next: function()
            {
              n += 1;

              if (n == 5)
                done = true;

              return { value: n, done: done };
            }
          };
        }
      };
      (iterableObject);
    )__";
  }

  duk_eval_string(ctx, containerCode);
  auto range = duk::get<TestType>(ctx, -1);

  auto iter = range.begin();
  REQUIRE(*iter == 0);

  REQUIRE(*(++iter) == 1);

  REQUIRE(*(iter++) == 1);
  REQUIRE(*iter == 2);

  REQUIRE(iter == iter);
  REQUIRE(iter != range.end());

  REQUIRE_THROWS_AS(*range.end(), duk::error);
}


TEST_CASE_METHOD(DukCppTest, "Ranges (std::random_access_iterator)")
{
  duk_eval_string(ctx_, "([0, 1, 2, 3, 4]);");
  auto range = duk::get<duk::safe_array_input_range<int>>(ctx_, -1);

  auto iter = range.begin();

  iter += 4;
  REQUIRE(*iter == 4);

  iter -= 2;
  REQUIRE(*iter == 2);

  REQUIRE(iter[-1] == 1);
  REQUIRE(iter[1] == 3);

  REQUIRE(*(iter - 1) == 1);
  REQUIRE(*(iter + 1) == 3);
  REQUIRE(*(1 + iter) == 3);

  REQUIRE(range.begin() < iter);
  REQUIRE(iter < range.end());
  REQUIRE(iter > range.begin());
  REQUIRE(iter >= range.begin());
  REQUIRE(iter >= iter);
  REQUIRE(iter <= range.end());
  REQUIRE(iter <= iter);

  REQUIRE(*(--iter) == 1);

  REQUIRE(*(iter--) == 1);
  REQUIRE(*iter == 0);

  REQUIRE(range.end() - range.begin() == 5);
  REQUIRE(range.begin() + 5 == range.end());
  REQUIRE(range.end() - 5 == range.begin());

  REQUIRE_THROWS_AS(*range.end(), duk::error);
  REQUIRE_THROWS_AS(*(--range.begin()), duk::error);
}


TEST_CASE_METHOD(DukCppTest, "Ranges (sum)")
{
  duk_push_global_object(ctx_);

  constexpr static auto sumRange = [](duk::safe_input_range<int> r)
  {
    return std::accumulate(r.begin(), r.end(), 0);
  };

  static constexpr auto sumRanges = [](duk::safe_input_range<int> r1, duk::safe_input_range<int> r2)
  {
    std::string result;

    for (auto iter1 = r1.begin(), iter2 = r2.begin(); iter1 != r1.end(); ++iter1, ++iter2)
      result += std::to_string(*iter1 + *iter2) + " ";

    return result;
  };

  duk::put_function<sumRange>(ctx_, -1, "sumRange");
  duk::put_function<sumRanges>(ctx_, -1, "sumRanges");

  duk_pop(ctx_); // Pop global object

  SECTION("Array")
  {
    duk_eval_string(ctx_, R"__(
      sumRange([1, 2, 3]);
    )__");
    REQUIRE(duk::get<int>(ctx_, -1) == 6);
    duk_pop(ctx_);

    duk_eval_string(ctx_, R"__(
      sumRanges([1, 2, 3], [10, 20, 30]);
    )__");
    REQUIRE(duk::get<std::string>(ctx_, -1) == "11 22 33 ");
    duk_pop(ctx_);
  }

  SECTION("Symbol.iterator")
  {
    duk_eval_string(ctx_, R"__(
      var iterableObject = {
        [Symbol.iterator]: function()
        {
          var n = 0;
          var done = false;

          return {
            next: function()
            {
              n += 1;

              if (n == 5)
                done = true;

              return { value: n, done: done };
            }
          };
        }
      };

      sumRange(iterableObject);
    )__");
    REQUIRE(duk::get<int>(ctx_, -1) == 10);
    duk_pop(ctx_);

    duk_eval_string(ctx_, R"__(
      sumRanges(iterableObject, iterableObject);
    )__");
    REQUIRE(duk::get<std::string>(ctx_, -1) == "2 4 6 8 ");
    duk_pop(ctx_);
  }
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
  REQUIRE(duk::get<std::string>(ctx_, -1) == "helloworld");
  duk_pop(ctx_);

  // Invalid arguments
  duk::push_function<f>(ctx_);
  duk::push(ctx_, A{});
  duk::push(ctx_, A{});
  REQUIRE_THROWS(duk_call(ctx_, 2));
  duk_pop(ctx_);
}
