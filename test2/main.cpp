#include <duk/context.h>
#include <duk/function.h>
#include <iostream>


int add(int a, int b)
{
  return a + b;
}

int stringN(int n, std::string_view s)
{
  for (int i = 0; i < n; ++i)
    std::cout << s << '\n';

  return 666;
}

void noargs()
{
  std::cout << "noargs\n";
}

void overloaded(int v)
{
  std::cout << "int\n";
}

void overloaded(std::string_view v)
{
  std::cout << "string_view\n";
}


int main()
{
  duk::context ctxOwning(duk_create_heap_default());
  auto ctx = ctxOwning.get();

  duk_push_global_object(ctx);
  duk::register_function<add>(ctx, -1, "add");
  duk::register_function<stringN>(ctx, -1, "stringN");
  duk::register_function<noargs>(ctx, -1, "noargs");
  duk::register_function<
    static_cast<void(*)(int)>(overloaded),
    static_cast<void(*)(std::string_view)>(overloaded)
  >(ctx, -1, "overloaded");
  duk_pop(ctx);

  //duk_eval_string(ctx, "add(2, 3)");
  //duk_eval_string(ctx, "stringN(10, 'test')");
  //duk_eval_string(ctx, "noargs()");
  duk_eval_string(ctx, "overloaded(5)");
  //duk_eval_string(ctx, "overloaded('string')");
  

  //std::cout << duk_get_int(ctx, -1) << '\n';
}
