#ifndef DUKCPP_DETAIL_ANY_H
#define DUKCPP_DETAIL_ANY_H

#include <type_traits>
#include <utility>
#include <vector>


namespace duk::detail
{


struct bad_any_cast : public std::bad_cast
{
  const char* what() const noexcept override
  {
    return "bad_any_cast";
  }
};


template<typename Allocator = std::allocator<std::byte>>
class any final
{
  using Deleter = void(*)(void*);

public:
  any(auto&& obj, const Allocator& alloc = Allocator()) :
    buffer_(sizeof(std::decay_t<decltype(obj)>), alloc),
    typeId_(type_id<std::decay_t<decltype(obj)>>()),
    deleter_(
      [](void* buffer)
      {
        using T = std::decay_t<decltype(obj)>;

        static_cast<T*>(buffer)->~T();
      }
    )
  {
    using T = std::decay_t<decltype(obj)>;

    new(buffer_.data()) T(std::forward<decltype(obj)>(obj));
  }

  ~any()
  {
    deleter_(buffer_.data());
  }

  any(const any&) = delete;
  any(any&&) = delete;

  any& operator=(const any&) = delete;
  any& operator=(any&&) = delete;

  /*
  TODO: Use this version when deducing this is available.

  template<typename U, typename Self>
  [[nodiscard]]
  U cast(this Self&& self)
  {
    using DecayU = std::decay_t<U>;

    static constexpr bool isConst = std::is_const_v<std::remove_reference_t<Self>>;

    if (self.typeId_ != type_id<std::decay_t<DecayU>>())
      throw bad_any_cast();

    return *static_cast<std::conditional_t<isConst, const DecayU*, DecayU*>>(
      static_cast<std::conditional_t<isConst, const void*, void*>>(
        self.buffer_.data()
      )
    );
  }
  */

  template<typename U>
  [[nodiscard]]
  U cast()
  {
    using DecayU = std::decay_t<U>;

    if (typeId_ != type_id<std::decay_t<DecayU>>())
      throw bad_any_cast();

    return *static_cast<DecayU*>(static_cast<void*>(buffer_.data()));
  }

  template<typename U>
  [[nodiscard]]
  U cast() const
  {
    using DecayU = std::decay_t<U>;

    if (typeId_ != type_id<std::decay_t<DecayU>>())
      throw bad_any_cast();

    return *static_cast<const DecayU*>(static_cast<const void*>(buffer_.data()));
  }

private:
  std::vector<std::byte, Allocator> buffer_;
  std::size_t typeId_;
  Deleter deleter_;
};


} // namespace duk::detail


#endif // DUKCPP_DETAIL_ANY_H
