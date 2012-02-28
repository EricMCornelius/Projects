#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include <ConnectionManager.h>
#include <Connection.h>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sys = boost::system;

// listens on a specific port and establishes connection objects
class Server {
public:
  typedef ConnectionManager::ConnectionPtr ConnectionPtr;

  Server(asio::io_service& loop) : _loop(loop), _acceptor(loop), _connections(loop), _backoff(loop) { }

  void listen(int port) {
    ip::tcp::endpoint endpoint(ip::tcp::v4(), port);
    _acceptor.open(endpoint.protocol());
    _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    _acceptor.bind(endpoint);
    _acceptor.listen();

    async_accept();
  }

  void async_accept() {
    // retrieve connection object from the manager
    ConnectionPtr connPtr = _connections.allocate();

    // if we failed to get a connection object (nullptr) then wait and try again
    if (connPtr)
      _acceptor.async_accept(connPtr->socket(), [=](const sys::error_code& ec){ accept_handler(connPtr, ec); });
    else {
      std::cout << "Unable to allocate connection object" << std::endl;

      // back off while waiting for a connection object to be freed up
      _backoff.expires_from_now(boost::posix_time::seconds(1));
      _backoff.async_wait([&](const sys::error_code& ec){ async_accept(); });
    }
  }

  void accept_handler(ConnectionPtr connPtr, const sys::error_code& ec) {
    // if we have an error, release the connection - otherwise open it
    if (ec)
      connPtr->release();
    else
      connPtr->open();
    // queue another accept operation
    async_accept();
  }

private:
  asio::io_service& _loop;
  ip::tcp::acceptor _acceptor;
  ConnectionManager _connections;
  asio::deadline_timer _backoff;
};

#endif
