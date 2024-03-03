#include "options.h"
#include <exception>
#include <fstream>
#include <iostream>
#include <duk/duk.h>
#include <string>
#include <vector>


namespace
{


void errorHandler(void* udata, const char* message)
{
  throw std::runtime_error(message);
}


[[nodiscard]]
std::vector<char> readFile(const std::string& fileName)
{
  auto file = std::ifstream(fileName, std::ios::binary | std::ios::ate);
  auto fileSize = file.tellg();
  auto data = std::vector<char>(fileSize);

  file.seekg(0);
  file.read(data.data(), fileSize);

  return data;
}


void print(const char* text)
{
  std::cout << text;
}


std::string duktapeErrorToString(duk_context* ctx, duk_idx_t objIdx)
{
  std::string name("-");
  std::string message("-");
  std::string fileName("-");
  std::string line("-");

  if(duk_get_prop_string(ctx, objIdx, "name"))
    name = duk_get_string(ctx, -1);
  duk_pop(ctx);

  if(duk_get_prop_string(ctx, objIdx, "message"))
    message = duk_get_string(ctx, -1);
  duk_pop(ctx);

  if(duk_get_prop_string(ctx, objIdx, "fileName"))
    fileName = duk_get_string(ctx, -1);
  duk_pop(ctx);

  if(duk_get_prop_string(ctx, objIdx, "lineNumber"))
    line = std::to_string(duk_get_int(ctx, -1));
  duk_pop(ctx);

  return fileName + "(" + line + "): " + name + ": " + message;
}


} // namespace


int main(int argc, char* argv[])
{
  try
  {
    auto options = parseOptions(argc, argv);
    auto ctx = duk::context(duk_create_heap(nullptr, nullptr, nullptr, nullptr, errorHandler));
    auto source = readFile(options.input);

    duk_push_global_object(ctx);
    duk::put_function<print>(ctx, -1, "print");
    duk_pop(ctx);

    duk_push_lstring(ctx, options.input.data(), options.input.size());
    if (duk_pcompile_lstring_filename(ctx, 0, source.data(), source.size()) != 0)
    {
      throw std::runtime_error(duktapeErrorToString(ctx, -1));
    }
    else
    {
      if (duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS)
        throw std::runtime_error(duktapeErrorToString(ctx, -1));
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "Unknown error\n";
    return 1;
  }
}
