#ifndef DUKCPP_TEST_LIFETIME_H
#define DUKCPP_TEST_LIFETIME_H

#include <duk/callable.h>


// Lifetime

struct Lifetime
{
  struct Observer final
  {
    using Counter = unsigned long long;

    [[nodiscard]]
    bool countersWithinLimits(const Observer& other) const noexcept
    {
      return ctorCount <= other.ctorCount &&
             dtorCount <= other.dtorCount &&
             copyCtorCount <= other.copyCtorCount &&
             moveCtorCount <= other.moveCtorCount &&
             copyAssignmentCount <= other.copyAssignmentCount &&
             moveAssignmentCount <= other.moveAssignmentCount;
    }

    [[nodiscard]]
    bool ctorDtorCountMatch() const noexcept
    {
      return dtorCount == (ctorCount + copyCtorCount + moveCtorCount);
    }

    Counter ctorCount = 0;
    Counter dtorCount = 0;
    Counter copyCtorCount = 0;
    Counter moveCtorCount = 0;
    Counter copyAssignmentCount = 0;
    Counter moveAssignmentCount = 0;
  };

  Lifetime(Observer& observer) noexcept:
    observer(&observer)
  {
    observer.ctorCount++;
  }

  ~Lifetime() noexcept
  {
    observer->dtorCount++;
  }

  Lifetime(const Lifetime& lifetime) noexcept :
    observer(lifetime.observer)
  {
    observer->copyCtorCount++;
  }

  Lifetime(Lifetime&& lifetime) noexcept :
    observer(lifetime.observer)
  {
    observer->moveCtorCount++;
  }

  Lifetime& operator=(const Lifetime&) noexcept
  {
    observer->copyAssignmentCount++;

    return *this;
  }

  Lifetime& operator=(Lifetime&&) noexcept
  {
    observer->moveAssignmentCount++;

    return *this;
  }

  Observer* observer = nullptr;
};


// CallableLifetime

struct CallableLifetime : public Lifetime
{
  void operator()()
  {
  }
};


namespace duk
{

template<>
struct callable_traits<CallableLifetime>
{
  using type = CallableLifetime;
};

} // namespace duk


#endif // DUKCPP_TEST_LIFETIME_H
