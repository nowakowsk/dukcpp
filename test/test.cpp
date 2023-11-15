#include <catch2/catch_test_macros.hpp>
#include <duk/duk.h>


struct DukCppTest
{
  using allocator = duk::allocator_adapter<>;

  DukCppTest() :
    ctx_(duk_create_heap(allocator::alloc, allocator::realloc, allocator::free, &allocator_, nullptr))
  {
  }

  allocator allocator_;
  duk::context ctx_;
};


static int addFunc(int a, int b)
{
  return a + b;
}


static int addFuncRef(const int& a, const int& b)
{
  return a + b;
}


TEST_CASE_METHOD(DukCppTest, "Register function")
{
  duk_push_global_object(ctx_);
  duk::register_function<addFunc>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk_get_int(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register function (reference arguments)")
{
  duk_push_global_object(ctx_);
  duk::register_function<addFuncRef>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk_get_int(ctx_, -1) == 3);
}


TEST_CASE_METHOD(DukCppTest, "Register simple lambda")
{
  static constexpr auto addLambda = [](int a, int b)
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::register_function<addLambda>(ctx_, -1, "addLambda");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addLambda(1, 2)");
  REQUIRE(duk_get_int(ctx_, -1) == 3);
}


/*
TEST_CASE_METHOD(DukCppTest, "Register functor")
{
  static constexpr std::function<int(int, int)> functor(addFunc);

  duk_push_global_object(ctx_);
  duk::register_function<addFunc>(ctx_, -1, "addFunc");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addFunc(1, 2)");
  REQUIRE(duk_get_int(ctx_, -1) == 3);
}
*/


/*
TEST_CASE_METHOD(DukCppTest, "Register lambda with capture list")
{
  int a = 3;

  static constexpr auto addLambda = [a](int b)
  {
    return a + b;
  };

  duk_push_global_object(ctx_);
  duk::register_function<addLambda>(ctx_, -1, "addLambda");
  duk_pop(ctx_);

  duk_eval_string(ctx_, "addLambda(2)");
  REQUIRE(duk_get_int(ctx_, -1) == 3);
}
*/
