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

private:
  detail::string message_;
};


} // namespace duk


#endif // DUKCPP_ERROR_H
