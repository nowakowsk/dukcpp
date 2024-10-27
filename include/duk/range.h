#ifndef DUKCPP_RANGE_H
#define DUKCPP_RANGE_H

#include <duk/error.h>
#include <duk/safe_handle.h>
#include <duk/scoped_pop.h>
#include <iterator>
#include <ranges>
#include <variant>


namespace duk
{


namespace detail
{

template<typename T>
struct type_traits;

} // namespace detail


template<typename T>
class array_input_range;


template<typename T>
class array_input_iterator final
{
public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::random_access_iterator_tag;

  array_input_iterator() = default;

  [[nodiscard]]
  decltype(auto) operator*() const
  {
    return operator[](0);
  }

  array_input_iterator& operator++() noexcept
  {
    ++arrayIdx_;

    return *this;
  }

  array_input_iterator operator++(int) noexcept
  {
    auto copy = *this;

    operator++();

    return copy;
  }

  array_input_iterator& operator--() noexcept
  {
    --arrayIdx_;

    return *this;
  }

  array_input_iterator operator--(int) noexcept
  {
    auto copy = *this;

    operator--();

    return copy;
  }

  [[nodiscard]]
  friend array_input_iterator operator+(const array_input_iterator& iter, difference_type n) noexcept
  {
    return { iter.arrayHandle_, static_cast<duk_uarridx_t>(iter.arrayIdx_ + n) };
  }

  [[nodiscard]]
  friend array_input_iterator operator+(difference_type n, const array_input_iterator& iter) noexcept
  {
    return operator+(iter, n);
  }

  [[nodiscard]]
  friend array_input_iterator operator-(const array_input_iterator& iter, difference_type n) noexcept
  {
    return { iter.arrayHandle_, static_cast<duk_uarridx_t>(iter.arrayIdx_ - n) };
  }

  [[nodiscard]]
  friend difference_type operator-(const array_input_iterator& lhs, const array_input_iterator& rhs) noexcept
  {
    // NOTE: Ignoring possibility of iterators not pointing to the same array.

    return lhs.arrayIdx_ - rhs.arrayIdx_;
  }

  array_input_iterator& operator+=(difference_type n) noexcept
  {
    arrayIdx_ += n;

    return *this;
  }

  array_input_iterator& operator-=(difference_type n) noexcept
  {
    arrayIdx_ -= n;

    return *this;
  }

  decltype(auto) operator[](difference_type n) const
  {
    auto ctx = arrayHandle_.get().ctx;

    if (arrayHandle_.get().empty()) [[unlikely]]
      throw error(ctx, "invalid symbol iterator dereference (uninitialized)");

    scoped_pop _(ctx); // push
    arrayHandle_.get().push();

    scoped_pop __(ctx); // duk_get_prop_index
    if (!duk_get_prop_index(ctx, -1, arrayIdx_ + n)) [[unlikely]]
      throw error(ctx, "invalid array iterator dereference (out of range)");

    return detail::type_traits<T>::pull(ctx, -1);
  }

  // NOTE: In theory, operator== and != should be covered by <=>, but removing explicit definitions trips static_assert
  //       verifying whether array_input_iterator conforms to std::random_access_iterator requirements.
  [[nodiscard]]
  bool operator==(const array_input_iterator& other) const noexcept
  {
    return arrayHandle_ == other.arrayHandle_ &&
           arrayIdx_ == other.arrayIdx_;
  }

  [[nodiscard]]
  bool operator!=(const array_input_iterator& other) const noexcept
  {
    return !operator==(other);
  }

  [[nodiscard]]
  std::partial_ordering operator<=>(const array_input_iterator& other) const noexcept
  {
    if (arrayHandle_ != other.arrayHandle_)
      return std::partial_ordering::unordered;

    return std::partial_order(arrayIdx_, other.arrayIdx_);
  }

private:
  friend class array_input_range<T>;

  array_input_iterator(const safe_handle& arrayHandle, duk_uarridx_t arrayIdx) noexcept :
    arrayHandle_(arrayHandle),
    arrayIdx_(arrayIdx)
  {
  }

  safe_handle arrayHandle_;
  duk_uarridx_t arrayIdx_ = 0;
};


template<typename T>
class array_input_range final
{
public:
  array_input_range(duk_context* ctx, duk_idx_t objIdx) noexcept :
    arrayHandle_(handle(ctx, objIdx))
  {
  }

  [[nodiscard]]
  array_input_iterator<T> begin() const
  {
    return { arrayHandle_, 0 };
  }

  [[nodiscard]]
  array_input_iterator<T> end() const
  {
    auto ctx = arrayHandle_.get().ctx;

    scoped_pop _(ctx); // push
    arrayHandle_.get().push();

    return { arrayHandle_, static_cast<duk_uarridx_t>(duk_get_length(ctx, -1)) };
  }

private:
  safe_handle arrayHandle_;
};


template<typename T>
class symbol_input_range;


template<typename T>
class symbol_input_iterator final
{
public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::input_iterator_tag;

  symbol_input_iterator() = default;

  [[nodiscard]]
  decltype(auto) operator*() const
  {
    auto ctx = containerHandle_.get().ctx;

    if (end_) [[unlikely]]
      throw error(ctx, "invalid symbol iterator dereference (out of range)");

    if (currHandle_.get().empty()) [[unlikely]]
      throw error(ctx, "invalid symbol iterator dereference (uninitialized)");

    scoped_pop _(ctx); // push
    currHandle_.get().push();

    scoped_pop __(ctx); // duk_get_prop_string
    if (!duk_get_prop_literal(ctx, -1, "value")) [[unlikely]]
      throw error(ctx, "invalid symbol iterator dereference ('value' property missing)");

    return detail::type_traits<T>::pull(ctx, -1);
  }

  symbol_input_iterator& operator++()
  {
    getNextValue();

    return *this;
  }

  symbol_input_iterator operator++(int)
  {
    auto copy = *this;

    operator++();
  
    return copy;
  }

  [[nodiscard]]
  bool operator==(const symbol_input_iterator& other) const noexcept
  {
    return (end_ && other.end_ && containerHandle_ == other.containerHandle_) ||
           iteratorHandle_ == other.iteratorHandle_;
  }

  [[nodiscard]]
  bool operator!=(const symbol_input_iterator& other) const noexcept
  {
    return !operator==(other);
  }

private:
  friend class symbol_input_range<T>;

  symbol_input_iterator(const safe_handle& containerHandle) noexcept :
    containerHandle_(containerHandle),
    end_(true)
  {
  }

  symbol_input_iterator(const safe_handle& containerHandle, const safe_handle& iteratorHandle) :
    containerHandle_(containerHandle),
    iteratorHandle_(iteratorHandle),
    end_(false)
  {
    getNextValue();
  }

  void getNextValue()
  {
    if (end_) [[unlikely]]
      return;

    auto ctx = iteratorHandle_.get().ctx;

    scoped_pop _(ctx); // push
    iteratorHandle_.get().push();

    duk_push_string(ctx, "next");

    scoped_pop __(ctx); // duk_call_prop
    duk_call_prop(ctx, -2, 0);

    if (!duk_is_object(ctx, -1)) [[unlikely]]
      throw error(ctx, "symbol iterator returned invalid value");

    currHandle_ = handle(ctx, -1);

    scoped_pop ___(ctx); // duk_get_prop_string
    if (duk_get_prop_string(ctx, -1, "done"))
      end_ = duk_get_boolean(ctx, -1);
  }

  safe_handle containerHandle_;
  safe_handle iteratorHandle_;
  safe_handle currHandle_;
  bool end_ = true;
};


template<typename T>
class symbol_input_range final
{
public:
  symbol_input_range(duk_context* ctx, duk_idx_t idx) noexcept :
    containerHandle_(handle(ctx, idx))
  {
  }

  [[nodiscard]]
  symbol_input_iterator<T> begin() const
  {
    auto ctx = containerHandle_.get().ctx;

    scoped_pop _(ctx); // push
    containerHandle_.get().push();

    duk_push_string(ctx, DUK_WELLKNOWN_SYMBOL("Symbol.iterator"));

    scoped_pop __(ctx); // duk_call_prop
    duk_call_prop(ctx, -2, 0);

    return { containerHandle_, handle(ctx, -1) };
  }

  [[nodiscard]]
  symbol_input_iterator<T> end() const
  {
    return { containerHandle_ };
  }

private:
  safe_handle containerHandle_;
};


template<typename T>
class input_range;


template<typename T>
class input_iterator final
{
public:
  using difference_type = std::ptrdiff_t;
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::input_iterator_tag;

  input_iterator() = default;

  [[nodiscard]]
  decltype(auto) operator*() const
  {
    return std::visit([](auto&& iter) { return *iter; }, impl_);
  }

  input_iterator& operator++()
  {
    std::visit([](auto&& iter) { ++iter; }, impl_);

    return *this;
  }

  input_iterator operator++(int)
  {
    auto copy = *this;

    operator++();

    return copy;
  }

  [[nodiscard]]
  bool operator==(const input_iterator& other) const
  {
    return impl_ == other.impl_;
  }

  [[nodiscard]]
  bool operator!=(const input_iterator& other) const
  {
    return !operator==(other);
  }

private:
  friend class input_range<T>;

  using input_iterator_impl = std::variant<
    array_input_iterator<T>,
    symbol_input_iterator<T>
  >;

  input_iterator(input_iterator_impl impl) :
    impl_(std::move(impl))
  {
  }

  input_iterator_impl impl_;
};


template<typename T>
class input_range final
{
private:
  using input_range_impl = std::variant<
    array_input_range<T>,
    symbol_input_range<T>
  >;

public:
  input_range(duk_context* ctx, duk_idx_t idx) :
    impl_(
      [&]() -> input_range_impl
      {
        if(duk_is_array(ctx, idx))
          return array_input_range<T>(ctx, idx);
        else
          return symbol_input_range<T>(ctx, idx);
      }()
    )
  {
  }

  [[nodiscard]]
  input_iterator<T> begin() const
  {
    return std::visit(
      [](auto&& range)
      {
        return typename input_iterator<T>::input_iterator_impl(range.begin());
      },
      impl_
    );
  }

  [[nodiscard]]
  input_iterator<T> end() const
  {
    return std::visit(
      [](auto&& range)
      {
        return typename input_iterator<T>::input_iterator_impl(range.end());
      },
      impl_
    );
  }

private:
  input_range_impl impl_;
};


} // namespace duk


#endif // DUKCPP_RANGE_H
