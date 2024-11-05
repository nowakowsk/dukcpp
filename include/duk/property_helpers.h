#ifndef DUKCPP_PROPERTY_HELPERS_H
#define DUKCPP_PROPERTY_HELPERS_H

#include <boost/callable_traits.hpp>
#include <duk/common.h>


namespace duk
{


namespace detail
{


template<auto member_ptr>
[[nodiscard]]
auto& property_getter(boost::callable_traits::class_of_t<decltype(member_ptr)>& obj)
{
  return obj.*member_ptr;
}


template<auto member_ptr>
void property_setter(
  boost::callable_traits::class_of_t<decltype(member_ptr)>& obj,
  member_type_t<decltype(member_ptr)>&& value
)
{
  obj.*member_ptr = std::move(value);
}


template<auto accessor>
duk_idx_t push_accessor(duk_context* ctx)
{
  static constexpr auto accessorWrapper = 
    detail::FunctionSignatureWrapper<function_descriptor<accessor, decltype(accessor)>, true>::run;

  return duk_push_c_function(ctx, accessorWrapper, DUK_VARARGS);
}


} // namespace detail


template<auto member_ptr>
void put_read_only_property(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  duk_push_lstring(ctx, name.data(), name.length());
  detail::push_accessor<detail::property_getter<member_ptr>>(ctx);
  duk_def_prop(ctx, idx - 2, DUK_DEFPROP_HAVE_GETTER);
}


template<auto member_ptr>
void put_property(duk_context* ctx, duk_idx_t idx, std::string_view name)
{
  duk_push_lstring(ctx, name.data(), name.length());
  detail::push_accessor<detail::property_getter<member_ptr>>(ctx);
  detail::push_accessor<detail::property_setter<member_ptr>>(ctx);
  duk_def_prop(ctx, idx - 3, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
}


} // namespace duk


#endif // DUKCPP_PROPERTY_HELPERS_H
