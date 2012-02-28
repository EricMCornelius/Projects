/*
 * Connection.h
 *
 *  Created on: Feb 24, 2012
 *      Author: corneliu
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <string>
#include <functional>
#include <vector>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sys = boost::system;

// represents a logical connection between endpoints
template <typename RequestProcessor>
class Connection {
public:
  Connection(asio::io_service& loop, size_t id)
    : _loop(loop), _socket(loop), _id(id), _in_use(true) {

    _requestProcessor.writeCb = [&](){ this->async_write(); };
  }

  // asio socket handle - needs to be manipulated externally
  ip::tcp::socket& socket() {
    return _socket;
  }

  // recycle the connection object and update the id
  void reinitialize(size_t id) {
    _socket = ip::tcp::socket(_loop);
    _in_use = true;
    _id = id;
    _requestProcessor.reset();
  }

  // opens the connection - called after server acceptor receives connection request
  void open() {
    std::cout << "Opening connection #" << _id << std::endl;
    std::cout << "Peer: " << _socket.remote_endpoint().address() << std::endl;

    // remote endpoint is stored, since it is invalidated when peer disconnects
    _endpoint = _socket.remote_endpoint();

    async_read();
  }

  void async_read() {
    _socket.async_read_some(_requestProcessor.inputBuffer().next(), [&](const sys::error_code& ec, size_t len) { this->read_handler(ec, len); });
  }

  void read_handler(const sys::error_code& ec, size_t len) {
    if (ec || !len) {
      close();
      return;
    }

    _requestProcessor.advanceInput(len);
    async_read();
  }

  void async_write() {
    _socket.async_write_some(_requestProcessor.outputBuffer().next(), [&](const sys::error_code& ec, size_t len) { this->write_handler(ec, len); });
  }

  void write_handler(const sys::error_code& ec, size_t len) {
    if (ec) {
      close();
      return;
    }

    if (len == 0)
      return;

    _requestProcessor.advanceOutput(len);
    async_write();
  }

  // closes the connection
  void close() {
    std::cout << "Closing connection #" << _id << std::endl;
    std::cout << "Peer: " << _endpoint.address() << std::endl;

    release();
  }

  // releases the connection object to allow subsequent re-use
  void release() {
    _in_use = false;
  }

  // current id value
  size_t id() const {
    return _id;
  }

  // in use flag - detemines whether object may be recycled
  bool in_use() const {
    return _in_use;
  }

private:
  asio::io_service& _loop;
  ip::tcp::socket _socket;
  ip::tcp::endpoint _endpoint;
  size_t _id;
  bool _in_use;
  RequestProcessor _requestProcessor;
};

#endif /* CONNECTION_H_ */
