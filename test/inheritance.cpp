#include "inheritance.h"
#include <duk/duk.h>


namespace duk
{


// InhBase
void* class_traits<InhBase>::prototype_heapptr([[maybe_unused]] duk_context* ctx)
{
  return prototype;
}

void* class_traits<InhBase>::prototype = nullptr;


// InhDer
void* class_traits<InhDer>::prototype_heapptr([[maybe_unused]] duk_context* ctx)
{
  return prototype;
}

void* class_traits<InhDer>::prototype = nullptr;


// InhFinal
void* class_traits<InhFinal>::prototype_heapptr([[maybe_unused]] duk_context* ctx)
{
  return prototype;
}

void* class_traits<InhFinal>::prototype = nullptr;


} // namespace duk


void registerInhBase(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<InhBase>
  >(ctx);

  duk_push_object(ctx);

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::put_function<&InhBase::methodA>(ctx, -1, "methodA");

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "InhBase");

  duk::class_traits<InhBase>::prototype = prototypeHandle;
}


void registerInhDer(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<InhDer>
  >(ctx);

  duk_push_object(ctx);

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::put_function<&InhDer::methodB>(ctx, -1, "methodB");

  duk_push_heapptr(ctx, duk::class_traits<InhBase>::prototype);
  duk_set_prototype(ctx, -2);

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "InhDer");

  duk::class_traits<InhDer>::prototype = prototypeHandle;
}


void registerInhFinal(duk_context* ctx, duk_idx_t idx)
{
  duk::push_function<
    duk::ctor<InhFinal>
  >(ctx);

  duk_push_object(ctx);

  auto prototypeHandle = duk_get_heapptr(ctx, -1);

  duk::put_function<&InhFinal::methodC>(ctx, -1, "methodC");

  duk_push_heapptr(ctx, duk::class_traits<InhDer>::prototype);
  duk_set_prototype(ctx, -2);

  duk_put_prop_string(ctx, -2, "prototype");

  duk_put_prop_string(ctx, idx - 1, "InhFinal");

  duk::class_traits<InhFinal>::prototype = prototypeHandle;
}
