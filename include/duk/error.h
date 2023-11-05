#ifndef DUKCPP_ERROR_H
#define DUKCPP_ERROR_H

#include <stdexcept>
#include <string>


namespace duk
{


class error : public std::runtime_error
{
public:
  error(const std::string& what);
  error(const char* what);
};


} // namespace duk

#endif // DUKCPP_ERROR_H
