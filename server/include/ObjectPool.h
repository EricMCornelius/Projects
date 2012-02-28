/*
 * ObjectPool.h
 *
 *  Created on: Feb 24, 2012
 *      Author: corneliu
 */

#ifndef OBJECTPOOL_H_
#define OBJECTPOOL_H_

#include <vector>
#include <iterator>
#include <cassert>

#include <Utilities.h>

template<typename Element, size_t BufferSize = 4096>
struct Pool {
  typedef std::vector<Element> Buffer;
  typedef std::vector<Buffer> BufferChain;

  static_assert(integral_log2(BufferSize) > 0, "Buffer size must be a power of 2");

  Pool() : _size(0) {
    // place the initial buffer in the list
    buffers.emplace_back(Buffer(BufferSize));
  }

  Pool(Pool&& other) : buffers(std::move(other.buffers)), _size(other._size) {

  }

public:

  Element& allocate(size_t index) {
    // buffer size is guaranteed to be power of two, so get the proper buffer
    // and element indices quickly via shift and bitwise and operations
    const size_t buffer_idx = index >> integral_log2(BufferSize);
    const size_t elem_idx = index & (BufferSize - 1);

    // if we're requesting an element beyond the total number of buffers - expand buffer chain
    if (buffer_idx >= buffers.size()) {
      // allocate new buffer chain using power of two growth factor
      BufferChain tmp(2 * buffer_idx);

      // swap new buffer chain into buffers array
      buffers.swap(tmp);

      // swap each element from tmp back into new buffer chain
      for (size_t i = 0; i < tmp.size(); ++i)
        buffers[i].swap(tmp[i]);

      // resize the new buffers at the end of the chain to the proper number of elements
      for (size_t i = tmp.size(); i < buffers.size(); ++i)
        buffers[i].resize(BufferSize);
    }

    return buffers[buffer_idx][elem_idx];
  }

  const Element* lookup_ptr(size_t index) const {
    // buffer size is guaranteed to be power of two, so get the proper buffer
    // and element indices quickly via shift and bitwise and operations
    size_t buffer_idx = index >> integral_log2(BufferSize);
    size_t elem_idx = index & (BufferSize - 1);

    if (buffer_idx >= buffers.size())
      return 0;

    return &buffers[buffer_idx][elem_idx];
  }

  Element& lookup_ref(size_t index) {
    // buffer size is guaranteed to be power of two, so get the proper buffer
    // and element indices quickly via shift and bitwise and operations
    const size_t buffer_idx = index >> integral_log2(BufferSize);
    const size_t elem_idx = index & (BufferSize - 1);

    // unchecked lookup
    return buffers[buffer_idx][elem_idx];
  }

  // return corresponding element from the pool
  Element& operator [](size_t index) {
    return lookup_ref(index);
  }

  // return corresponding element from the pool
  const Element& operator [](size_t index) const {
    const Element* ptr = lookup_ptr(index);
    assert(ptr);
    return *ptr;
  }

  class PoolIterator: public std::iterator<std::input_iterator_tag, Element> {
  public:
    PoolIterator(Pool& pool_, const size_t current_ = 0) :
        pool(pool_), current(current_) {
    }

    PoolIterator operator ++() {
      ++current;
      return *this;
    }

    PoolIterator operator ++(int) {
      PoolIterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator ==(const PoolIterator& rhs) {
      return (&pool == &rhs.pool && current == rhs.current);
    }

    bool operator !=(const PoolIterator& rhs) {
      return !(*this == rhs);
    }

    Element& operator*() {
      return pool.lookup_ref(current);
    }

    Element* operator ->() {
      return pool.lookup_ptr(current);
    }

  private:
    Pool& pool;
    size_t current;
  };

  PoolIterator begin() {
    return PoolIterator(*this, 0);
  }

  PoolIterator end() {
    return PoolIterator(*this, _size);
  }

  virtual size_t size() const {
    return _size;
  }

  // reset the allocation position to the start of the pool
  virtual void reset() {
    _size = 0;
  }

  template <typename Container>
  void copy(Container& out) const {
    std::copy(begin(), end(), out.begin());
  }

  void copy(std::vector<Element>& out) const {
    out.reserve(_size);
    std::copy(begin(), end(), out.begin());
  }

  // buffer chain holds all currently allocated elements
  BufferChain buffers;

private:
  Pool(const Pool& other);

  // keeps track of size of the pool
  size_t _size;
};

#endif /* OBJECTPOOL_H_ */
