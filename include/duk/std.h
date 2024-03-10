#ifndef DUKCPP_STD_H
#define DUKCPP_STD_H

#include <duk/allocator.h>
#include <string>
#include <utility>


namespace duk
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


} // namespace duk


#endif // DUKCPP_STD_H
