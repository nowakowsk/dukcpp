#ifndef DUKCPP_PROPERTY_HELPERS_H
#define DUKCPP_PROPERTY_HELPERS_H

#include <boost/callable_traits.hpp>
#include <duk/common.h>


namespace duk
{


namespace detail
{


template<auto MemberPtr>
[[nodiscard]]
auto& prop_getter(boost::callable_traits::class_of_t<decltype(MemberPtr)>& obj)
{
  return obj.*MemberPtr;
}


template<auto MemberPtr>
void prop_setter(
  boost::callable_traits::class_of_t<decltype(MemberPtr)>& obj,
  const member_type_t<decltype(MemberPtr)>& value
)
{
  obj.*MemberPtr = value;
}


template<auto Accessor>
duk_idx_t push_prop_accessor(duk_context* ctx)
{
  return duk_push_c_function(
    ctx,
    detail::FunctionSignatureWrapper<function_descriptor<Accessor, decltype(Accessor)>, true>::run,
    DUK_VARARGS
  );
}


} // namespace detail


template<auto MemberPtr>
duk_idx_t push_prop_getter(duk_context* ctx)
{
  return detail::push_prop_accessor<detail::prop_getter<MemberPtr>>(ctx);
}


template<auto MemberPtr>
duk_idx_t push_prop_setter(duk_context* ctx)
{
  return detail::push_prop_accessor<detail::prop_setter<MemberPtr>>(ctx);
}


template<auto MemberPtr>
void def_prop_getter(duk_context* ctx, duk_idx_t idx, std::string_view name, duk_uint_t flags = 0)
{
  duk_push_lstring(ctx, name.data(), name.length());
  push_prop_getter<MemberPtr>(ctx);
  duk_def_prop(ctx, idx - 2, DUK_DEFPROP_HAVE_GETTER | flags);
}


template<auto MemberPtr>
void def_prop_setter(duk_context* ctx, duk_idx_t idx, std::string_view name, duk_uint_t flags = 0)
{
  duk_push_lstring(ctx, name.data(), name.length());
  push_prop_setter<MemberPtr>(ctx);
  duk_def_prop(ctx, idx - 2, DUK_DEFPROP_HAVE_SETTER | flags);
}


template<auto MemberPtr>
void def_prop(duk_context* ctx, duk_idx_t idx, std::string_view name, duk_uint_t flags = 0)
{
  duk_push_lstring(ctx, name.data(), name.length());
  push_prop_getter<MemberPtr>(ctx);
  push_prop_setter<MemberPtr>(ctx);
  duk_def_prop(ctx, idx - 3, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | flags);
}


} // namespace duk


#endif // DUKCPP_PROPERTY_HELPERS_H
