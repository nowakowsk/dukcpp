#ifndef DUKCPP_ITERABLE_H
#define DUKCPP_ITERABLE_H

#include <duk/function_helpers.h>
#include <duk/safe_handle.h>
#include <duk/type_traits_helpers.h>
#include <ranges>


namespace duk
{


// Used for marking types to be treated as Objects supporting iterable protocol (Symbol.iterator) in ES.
template<typename T>
struct iterable_traits_type;


// iterable

template<typename T>
concept iterable = requires
{
  typename iterable_traits_type<T>::type;
};


template<typename T>
using iterable_traits_type_t = iterable_traits_type<T>::type;


// as_iterable

template<typename T>
struct as_iterable
{
};


template<typename T>
struct iterable_traits_type<as_iterable<T>>
{
  using type = T;
};


// make_iterable

template<typename T>
void make_iterable(duk_context* ctx, duk_idx_t idx)
{
  put_prop_function(ctx, idx, DUK_WELLKNOWN_SYMBOL("Symbol.iterator"),
    [ctx]()
    {
      duk_push_this(ctx);
      auto hnd = safe_handle({ctx, -1});
      auto range = std::ranges::subrange(get<T&>(hnd.ctx(), -1));

      duk_push_object(ctx);
      put_prop_function(ctx, -1, "next",
        [hnd, range]() mutable
        {
          duk_push_object(hnd.ctx());
          put_prop(hnd.ctx(), -1, "done", range.begin() == range.end());
          if (range.begin() != range.end())
          {
            put_prop(hnd.ctx(), -1, "value", *range.begin());
            range.advance(1);
          }
          return handle(hnd.ctx(), -1);
        }
      );
      return handle(hnd.ctx(), -1);
    }
  );
}


} // namespace duk


#endif // DUKCPP_ITERABLE_H
