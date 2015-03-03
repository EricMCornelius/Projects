#pragma once

#define BOOST_CB_DISABLE_DEBUG 1

#include <segment_cache.hpp>

#include <string>
#include <unordered_map>
#include <array>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>

template <std::size_t block_size = 4096>
struct ring_buffer_impl {
  struct metadata {
    uint64_t id;
    uint64_t offset;
    uint32_t size;
  };

  typedef std::array<uint8_t, block_size> data;

  struct message {
    uint8_t* ptr;
    uint32_t size;

    std::string string() {
      return {reinterpret_cast<const char*>(ptr), size};
    }
  };

  typedef boost::interprocess::allocator<metadata, boost::interprocess::managed_mapped_file::segment_manager> metadata_allocator;
  typedef boost::interprocess::allocator<uint8_t, boost::interprocess::managed_mapped_file::segment_manager> data_allocator;
  typedef boost::circular_buffer<metadata, metadata_allocator> metadata_buffer;
  typedef boost::circular_buffer<data, data_allocator> data_buffer;

  template <typename T1, typename T2>
  struct range_adaptor_impl {
    range_adaptor_impl(std::pair<T1, T2>& pair) : _pair(pair) {}

    auto begin() {
      return _pair.first;
    }

    auto end() {
      return _pair.second;
    }

    std::pair<T1, T2>& _pair;
  };

  template <typename T1, typename T2>
  auto range_adaptor(std::pair<T1, T2>& p) {
    return range_adaptor_impl<T1, T2>(p);
  }

  ring_buffer_impl(const std::string& name, std::size_t num_elements = 1024) {
    using namespace boost;
    using namespace boost::interprocess;

    auto buffer_space = (sizeof(metadata) + block_size) * num_elements + sizeof(metadata_buffer) + sizeof(data_buffer);
    auto segment_size = (ceil(buffer_space / mapped_region::get_page_size()) + 1) * mapped_region::get_page_size();
    auto& segment = segment_cache::global().get(name, segment_size);

    metadata_allocator metadata_alloc(segment->get_segment_manager());
    _metadata = segment->find_or_construct<metadata_buffer>("metadata")(num_elements, metadata_alloc);
    if (!_metadata)
      throw std::runtime_error("could not allocate metadata buffer");

    data_allocator data_alloc(segment->get_segment_manager());
    _data = segment->find_or_construct<data_buffer>("data")(num_elements * block_size, data_alloc);
    if (!_data)
      throw std::runtime_error("could not allocate data buffer");

    if (!_metadata->empty()) {
      auto last = _metadata->back();
      _next = {last.id + 1, last.offset + last.size, 0};
    }
    else {
      _next = {0, 0, 0};
    }
  }

  void push(message& msg) {
    push(msg.ptr, msg.size);
  }

  void push(uint8_t* ptr, uint32_t size) {
    if (size > _data->capacity()) {
      throw std::bad_alloc();
    }
    auto remainder = _next.offset % _data->capacity();
    auto beyond_bound = (remainder + size) > _data->capacity();
    if (!beyond_bound) {
      _data->insert(std::end(*_data), ptr, ptr + size);
      auto end = &_data->back();
      _next.size = size;
      _metadata->push_back(_next);
      ++_next.id;
      _next.offset += size;
      _next.size = 0;
      auto min = _next.offset > _data->capacity() ? _next.offset - _data->capacity() : 0;
      while(!_metadata->empty() && _metadata->front().offset < min) {
        _metadata->pop_front();
      }
    }
    else {
      // empty pad to end of buffer
      _data->insert(std::end(*_data), _data->capacity() - remainder, 0);
      _next.offset += _data->capacity() - remainder;
      push(ptr, size);
    }
  }

  message get(uint64_t id) {
    auto res = std::equal_range(std::begin(*_metadata), std::end(*_metadata), metadata{id, 0}, [](const auto& m1, const auto& m2) { return m1.id < m2.id; });
    for (auto& val : range_adaptor(res)) {
      auto buf = _data->array_two().first.get();
      auto offset = val.offset % _data->capacity();
      return {buf + offset, val.size};
    }
    std::cout << "no matching id:" << id << std::endl;
    return {nullptr, 0};
  }

  void print() {
    for (const auto& part : *_metadata)
      std::cout << "id: " << part.id << " offset: " << part.offset << " size: " << part.size << std::endl;
  }

  metadata_buffer* _metadata;
  data_buffer* _data;
  metadata _next;
};

using ring_buffer = ring_buffer_impl<>;
