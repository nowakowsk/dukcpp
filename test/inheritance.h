#ifndef DUKCPP_TEST_INHERITANCE_H
#define DUKCPP_TEST_INHERITANCE_H

#include <duk/class.h>


struct InhBase
{
  virtual ~InhBase() = default;

  virtual const char* methodA() { return "BaseA"; }
};


struct InhDer : InhBase
{
  virtual ~InhDer() = default;

  const char* methodA() override { return "DerA"; }
  virtual const char* methodB() { return "DerB"; }
};


struct InhFinal : InhDer
{
  virtual ~InhFinal() = default;

  const char* methodA() override { return "FinalA"; }
  const char* methodB() override { return "FinalB"; }
  const char* methodC() { return "FinalC"; }
};


namespace duk
{


template<>
struct class_traits<InhBase>
{
  static void* prototype_heapptr(duk_context* ctx);

  static void* prototype;
};


template<>
struct class_traits<InhDer>
{
  using base = InhBase;

  static void* prototype_heapptr(duk_context* ctx);

  static void* prototype;
};


template<>
struct class_traits<InhFinal>
{
  using base = InhDer;

  static void* prototype_heapptr(duk_context* ctx);

  static void* prototype;
};


} // namespace duk


// Return prototype object handles
void registerInhBase(duk_context* ctx, duk_idx_t idx);
void registerInhDer(duk_context* ctx, duk_idx_t idx);
void registerInhFinal(duk_context* ctx, duk_idx_t idx);


#endif // DUKCPP_TEST_INHERITANCE_H
