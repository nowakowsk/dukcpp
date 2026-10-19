#include "common.h"
#include <catch2/catch_message.hpp>
#include <optional>


struct ESErrorInfo
{
  duk_errcode_t code = DUK_ERR_NONE;
  std::string message = "[no message]";
};


std::optional<ESErrorInfo> makeESErrorInfo(duk_context* ctx, duk_idx_t idx)
{
  if (!duk_is_error(ctx, idx))
    return {};

  ESErrorInfo info;

  info.code = duk_get_error_code(ctx, idx);

  if (duk_get_prop_string(ctx, idx, "message"))
    info.message = duk_get_string(ctx, -1);
  duk_pop(ctx);

  UNSCOPED_INFO("ES error detected (code: " << info.code <<
                ", message: \"" << info.message <<  "\").");

  return info;
}


bool isESError(duk_context* ctx, duk_idx_t idx)
{
  return makeESErrorInfo(ctx, idx) ? true : false;
}


bool isESError(duk_context* ctx, duk_idx_t idx, duk_errcode_t code)
{
  auto info = makeESErrorInfo(ctx, idx);

  return info && info->code == code;
}


bool isESError(duk_context* ctx, duk_idx_t idx, duk_errcode_t code, std::string_view message)
{
  auto info = makeESErrorInfo(ctx, idx);

  return info && info->code == code && info->message == message;
}


int add(int a, int b)
{
  return a + b;
}


int add(int a, int b, int c)
{
  return a + b + c;
}


std::string add(std::string_view a, std::string_view b)
{
  return std::string{a} + std::string{b};
}
