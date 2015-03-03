#pragma once

#include <segment_cache.hpp>

#include <string>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>

struct persistent_cursor {
  persistent_cursor(const std::string& name) : name(name) {
    using namespace boost;
    using namespace boost::interprocess;
    typedef typename interprocess::allocator<uint64_t, managed_mapped_file::segment_manager> cursor_allocator;

    auto& segment = segment_cache::global().get("cursors", 16 * 1024 * 1024);
    //cursor_allocator cursor_alloc(segment->get_segment_manager());

    value = segment->find_or_construct<uint64_t>(name.c_str())(0);
    if (!value)
      throw std::runtime_error("could not allocate cursor");
  }

  uint64_t& get() {
    return *value;
  }

  std::string name;
  uint64_t* value;
};
