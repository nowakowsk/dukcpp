#ifndef DUKCPP_FWD_H
#define DUKCPP_FWD_H

#include <duk/detail/std.h>
#include <duktape.h>


namespace duk
{


namespace detail
{


struct TypeTraitsOptions
{
  bool iterable = false;
};

template<typename T, TypeTraitsOptions Options = TypeTraitsOptions{}>
struct type_traits;

bool finalize_object(duk_context* ctx, duk_idx_t idx);
bool finalize_callable(duk_context* ctx, duk_idx_t idx);

template<typename T>
string classTraitsName(duk_context* ctx);

string typeName(duk_context* ctx, duk_idx_t idx);

bool clone(duk_context* ctx, duk_idx_t idx);


} // namespace detail


} // namespace duk


#endif // DUKCPP_FWD_H
