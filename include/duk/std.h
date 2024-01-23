#ifndef DUKCPP_STD_H
#define DUKCPP_STD_H

#include <duk/allocator.h>
#include <string>


namespace duk
{


// string

template<typename T>
using basic_string = std::basic_string<T, std::char_traits<T>, allocator<T>>;

using string = basic_string<char>;


} // namespace duk


#endif // DUKCPP_STD_H
