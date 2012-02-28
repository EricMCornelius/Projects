/*
 * ReadBuffer.h
 *
 *  Created on: Feb 24, 2012
 *      Author: corneliu
 */

#ifndef READBUFFER_H_
#define READBUFFER_H_

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <array>

#include <ObjectPool.h>

namespace asio = boost::asio;
namespace ip = boost::asio::ip;
namespace sys = boost::system;

template <size_t BufferSize = 4096>
class ReadBuffer : public Pool<char, BufferSize> {
public:
  typedef Pool<char, BufferSize> Base;
  typedef std::array<asio::mutable_buffer, 3> Chain;

  ReadBuffer() : _read(0) { }

  ReadBuffer(ReadBuffer&& other) : Base(std::forward<Base>(other)), _read(other._read) { }

  Chain next() {
    size_t buffer_idx = _read >> integral_log2(BufferSize);
    size_t elem_idx = _read & (BufferSize - 1);
    size_t left = BufferSize - elem_idx;

    Base::allocate(BufferSize * (buffer_idx + 2));

    Chain c;
    c[0] = asio::buffer(&Base::buffers[buffer_idx][elem_idx], left);
    c[1] = asio::buffer(Base::buffers[buffer_idx + 1]);
    c[2] = asio::buffer(Base::buffers[buffer_idx + 2]);

    return c;
  }

  void advance(size_t len) {
    _read += len;
  }

  void reset() {
    _read = 0;
  }

  size_t size() const {
    return _read;
  }

private:
  size_t _read;

  ReadBuffer(const ReadBuffer&);
};


#endif /* READBUFFER_H_ */
