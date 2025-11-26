#ifndef DUKCPP_DETAIL_STD_H
#define DUKCPP_DETAIL_STD_H

#include <duk/allocator.h>
#include <cstdio>
#include <string>
#include <utility>


namespace duk::detail
{


// string

template<typename T>
using basic_string = std::basic_string<T, std::char_traits<T>, allocator<T>>;

using string = basic_string<char>;

template<typename ...Args>
[[nodiscard]]
string make_string(duk_context* ctx, Args&&... args)
{
  return string(std::forward<Args>(args)..., allocator<char>(ctx));
}

[[nodiscard]]
static string to_string(duk_context* ctx, int value)
{
  int size = std::snprintf(nullptr, 0, "%d", value);
  if (size <= 0)
    return make_string(ctx);

  auto str = make_string(ctx, string::size_type(size), ' ');
  std::snprintf(str.data(), str.size() + 1, "%d", value);
  return str;
}


} // namespace duk::detail


#endif // DUKCPP_DETAIL_STD_H
