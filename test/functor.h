#ifndef DUKCPP_TEST_FUNCTOR_H
#define DUKCPP_TEST_FUNCTOR_H

#include <duk/callable.h>
#include <tuple>


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


Functor makeFunctor()
{
  return {};
}


namespace duk
{

template<>
struct callable_traits_type<Functor>
{
  using type = Functor;
};


template<>
struct callable_traits_signature_pack<Functor>
{
  using type = std::tuple<bool(), int(int)>;
};


} // namespace duk


#endif // DUKCPP_TEST_FUNCTOR_H
