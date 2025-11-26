#include "debugger.h"
#include "options.h"
#include <duk/duk.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>


namespace
{


void errorHandler([[maybe_unused]] void* udata, const char* message)
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


} // namespace


int main(int argc, char* argv[])
{
  auto ctx = duk::context(duk_create_heap(nullptr, nullptr, nullptr, nullptr, errorHandler));

  try
  {
    auto options = parseOptions(argc, argv);
    auto source = readFile(options.input);

    duk_push_global_object(ctx);
    duk::put_prop_function<print>(ctx, -1, "print");
    duk_pop(ctx);

    duk_push_string(ctx, options.input.c_str());
    if (duk_pcompile_lstring_filename(ctx, 0, source.data(), source.size()) != 0)
    {
      throw duk::es_error(ctx, -1);
    }
    else
    {
      std::optional<DebuggerContext> debugCtx;
      if (options.debug)
        debugCtx.emplace(ctx, options.debugPort);

      if (duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS)
        throw duk::es_error(ctx, -1);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << '\n' << e.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "\nUnknown error\n";
    return 1;
  }
}
