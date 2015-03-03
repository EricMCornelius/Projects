#pragma once

#include <string>
#include <unordered_map>
#include <array>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>

struct segment_cache {
  segment_cache(const std::string& prefix = "/tmp/") : prefix(prefix) {}

  auto& get(const std::string& name, std::size_t size) {
    using namespace boost::interprocess;

    auto lookup = cache.find(name);
    if (lookup == std::end(cache)) {
      auto path = prefix + name;
      std::cout << "loading segment: " << path << std::endl;
      lookup = cache.insert(lookup, std::make_pair(name, std::make_unique<managed_mapped_file>(open_or_create, path.c_str(), size)));
      auto& mapped_file = lookup->second;
      if (mapped_file->get_size() < size) {
        mapped_file->grow(path.c_str(), size - mapped_file->get_size());
      }
    }
    return lookup->second;
  }

  std::string prefix;
  std::unordered_map<std::string, std::unique_ptr<boost::interprocess::managed_mapped_file>> cache;

  static segment_cache& global(const std::string& prefix = "/tmp/") {
    static segment_cache global_cache(prefix);
    return global_cache;
  }
};
