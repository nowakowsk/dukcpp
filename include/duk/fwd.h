#ifndef DUKCPP_FWD_H
#define DUKCPP_FWD_H

#include <duktape.h>


namespace duk
{


namespace detail
{


template<typename T>
struct type_traits;

bool finalize_object(duk_context* ctx, duk_idx_t idx);
bool finalize_callable(duk_context* ctx, duk_idx_t idx);


} // namespace detail


} // namespace duk


#endif // DUKCPP_FWD_H
