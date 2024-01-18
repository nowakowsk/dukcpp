#ifndef DUKCPP_ERROR_H
#define DUKCPP_ERROR_H

#include <stdexcept>


namespace duk
{


class error : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};


} // namespace duk

#endif // DUKCPP_ERROR_H
