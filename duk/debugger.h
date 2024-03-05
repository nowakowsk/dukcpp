#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <boost/asio.hpp>
#include <duktape.h>


class DebuggerContext final
{
public:
  DebuggerContext(duk_context* dukCtx, unsigned short port);

private:
  boost::asio::io_context ioCtx_;
  boost::asio::ip::tcp::socket socket_;

  duk_size_t read(char* buffer, duk_size_t size);
  duk_size_t write(const char* buffer, duk_size_t size);
  void detached(duk_context* ctx);
};


#endif // DEBUGGER_H
