/*
 * WriteBuffer.h
 *
 *  Created on: Feb 24, 2012
 *      Author: corneliu
 */

#ifndef WRITEBUFFER_H_
#define WRITEBUFFER_H_

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <string>
#include <functional>
#include <vector>

#include <ObjectPool.h>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sys = boost::system;

template <size_t BufferSize = 4096>
class WriteBuffer : public Pool<char, BufferSize> {
public:
  typedef Pool<char, BufferSize> Base;
  typedef std::array<asio::mutable_buffer, 3> Chain;

  WriteBuffer() : _written(0) {

  }

  WriteBuffer(WriteBuffer&& other) : Base(std::forward<Base>(other)), _written(other._written) { }

  Chain next() {
    size_t buffer_idx = _written >> integral_log2(BufferSize);
    size_t elem_idx = _written & (BufferSize - 1);
    size_t left = BufferSize - elem_idx;

    Base::allocate(BufferSize * (buffer_idx + 2));

    Chain c;
    c[0] = asio::buffer(&Base::buffers[buffer_idx][elem_idx], left);
    c[1] = asio::buffer(Base::buffers[buffer_idx + 1]);
    c[2] = asio::buffer(Base::buffers[buffer_idx + 2]);

    return c;
  }

  void advance(size_t len) {
    _written += len;
  }

  void reset() {
    _written = 0;
  }

  size_t size() const {
    return _written;
  }

private:
  size_t _written;

  WriteBuffer(const WriteBuffer&);
};

#endif /* WRITEBUFFER_H_ */
