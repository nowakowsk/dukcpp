#ifndef DUKCPP_ERROR_H
#define DUKCPP_ERROR_H

#include <duk/allocator.h>
#include <duk/detail/std.h>
#include <duktape.h>
#include <stdexcept>
#include <string_view>


namespace duk
{


class error : public std::exception
{
public:
  error(duk_context* ctx, std::string_view message) :
    message_(detail::make_string(ctx, message))
  {
  }

  [[nodiscard]]
  const char* what() const noexcept override
  {
    return message_.c_str();
  }

protected:
  detail::string message_;
};


class es_error : public error
{
public:
  es_error(duk_context* ctx, duk_idx_t idx) :
    error(ctx, {}),
    name(detail::make_string(ctx)),
    message(detail::make_string(ctx)),
    filename(detail::make_string(ctx))
  {
    if (duk_get_prop_string(ctx, idx, "name"))
      name = detail::make_string(ctx, duk_get_string(ctx, -1));
    duk_pop(ctx);

    if (duk_get_prop_string(ctx, idx, "message"))
      message = detail::make_string(ctx, duk_get_string(ctx, -1));
    duk_pop(ctx);

    if (duk_get_prop_string(ctx, idx, "fileName"))
      filename = detail::make_string(ctx, duk_get_string(ctx, -1));
    duk_pop(ctx);

    if (duk_get_prop_string(ctx, idx, "lineNumber"))
      line = duk_get_int(ctx, -1);
    duk_pop(ctx);

    message_ = filename + "(" + detail::to_string(ctx, line) + "): " + name + ": " + message;
  }

  detail::string name;
  detail::string message;
  detail::string filename;
  duk_int_t line = 0;
};


} // namespace duk


#endif // DUKCPP_ERROR_H
