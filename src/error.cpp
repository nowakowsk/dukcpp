#include <duk/error.h>


namespace duk
{


error::error(const std::string& what) :
  std::runtime_error(what)
{
}


error::error(const char* what) :
  std::runtime_error(what)
{
}


} // namespace duk
