/*
 * BlockingQueue.h
 *
 *  Created on: Feb 25, 2012
 *      Author: corneliu
 */

#ifndef BLOCKINGQUEUE_H_
#define BLOCKINGQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <array>

#include <Utilities.h>

template <typename Element, size_t Size = 16>
class BlockingQueue {
public:

  static_assert(integral_log2(Size) > 0, "Buffer size must be a power of 2");

  BlockingQueue() : _start(0), _end(0) { }

  size_t mod(size_t val) {
    return val & (Size - 1);
  }

  void push(Element&& elem) {
    std::unique_lock<std::mutex> lock(_lock);
    _overflow.wait(lock, [&]{ return this->mod(_end + 1) != this->mod(_start); });

    _elements[mod(_end)] = std::move(elem);
    ++_end;
    _underflow.notify_one();
  }

  void pop(Element& elem) {
    std::unique_lock<std::mutex> lock(_lock);
    _underflow.wait(lock, [&]{ return _start != _end; });

    elem = std::move(_elements[mod(_start)]);
    ++_start;
    _overflow.notify_one();
  }

private:
  size_t _start;
  size_t _end;

  std::array<Element, Size> _elements;
  std::condition_variable _underflow;
  std::condition_variable _overflow;
  std::mutex _lock;
};


#endif /* BLOCKINGQUEUE_H_ */
