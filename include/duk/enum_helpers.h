#ifndef DUKCPP_ENUM_HELPERS_H
#define DUKCPP_ENUM_HELPERS_H

#include <duk/common.h>
#include <duk/type_traits_helpers.h>
#include <string_view>


namespace duk
{


namespace detail
{


// TODO: This would be better as a lambda in push_enum, but it raises an internal compiler error in MSVC.
// Bug report: https://developercommunity.visualstudio.com/t/ICE-when-recursively-calling-a-lambda-te/10794160
void push_enum_item(duk_context* ctx, enumeration auto value, std::string_view name, auto&&... enum_items)
{
  duk_push_lstring(ctx, name.data(), name.length());
  push(ctx, value);
  duk_def_prop(
    ctx, -3,
    DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE |
    DUK_DEFPROP_CLEAR_CONFIGURABLE
  );

  if constexpr (sizeof...(enum_items) != 0)
    push_enum_item(ctx, std::forward<decltype(enum_items)>(enum_items)...);
}


} // namespace detail


duk_idx_t push_enum(duk_context* ctx, auto&&... enum_items)
{
  auto idx = duk_push_object(ctx);

  detail::push_enum_item(ctx, std::forward<decltype(enum_items)>(enum_items)...);

  return idx;
}


void def_prop_enum(duk_context* ctx, duk_idx_t idx, std::string_view name, auto&&... enum_items)
{
  duk_push_lstring(ctx, name.data(), name.length());
  push_enum(ctx, std::forward<decltype(enum_items)>(enum_items)...);
  duk_def_prop(
    ctx, idx - 2,
    DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_CLEAR_ENUMERABLE |
    DUK_DEFPROP_CLEAR_CONFIGURABLE
  );
}


} // namespace duk


#endif // DUKCPP_ENUM_HELPERS_H
