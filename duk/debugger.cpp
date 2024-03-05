#include "debugger.h"
#include <iostream>
#include <stdexcept>


DebuggerContext::DebuggerContext(duk_context* dukCtx, unsigned short port) :
  socket_(ioCtx_)
{
  boost::asio::ip::tcp::acceptor acceptor(ioCtx_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

  std::cout << "Debugger waiting for connection (port: " << port << ")...\n";

  acceptor.accept(socket_);

  duk_debugger_attach(
    dukCtx,
    [](void* udata, char* buffer, duk_size_t size) -> duk_size_t
    {
      return static_cast<DebuggerContext*>(udata)->read(buffer, size);
    },
    [](void* udata, const char* buffer, duk_size_t size) -> duk_size_t
    {
      return static_cast<DebuggerContext*>(udata)->write(buffer, size);
    },
    nullptr, nullptr, nullptr, nullptr,
    [](duk_context* ctx, void* udata) -> void
    {
      static_cast<DebuggerContext*>(udata)->detached(ctx);
    },
    this
  );
}


duk_size_t DebuggerContext::read(char* buffer, duk_size_t size)
{
  if (!socket_.is_open())
    throw std::runtime_error("Duktape debugger detached.");

  boost::system::error_code error;
  auto bytesRead = socket_.read_some(boost::asio::buffer(buffer, size), error);

  if (error == boost::asio::stream_errc::eof)
    return 0;
  else if (error)
    throw boost::system::system_error(error);

  return bytesRead;
}


duk_size_t DebuggerContext::write(const char* buffer, duk_size_t size)
{
  if (!socket_.is_open())
    throw std::runtime_error("Duktape debugger detached.");

  return boost::asio::write(socket_, boost::asio::buffer(buffer, size));
}


void DebuggerContext::detached(duk_context* ctx)
{
  socket_.close();
}
