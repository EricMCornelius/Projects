#pragma once

#include <string>

struct Format {
  std::size_t width;
  std::size_t height;
  std::string description;
};

struct Device {
  std::string id;
  std::string name;
  std::string bus;
  std::vector<Format> formats;

  int handle;
  uint32_t capabilities;

  bool supports(uint32_t type) {
    return capabilities & type;
  }
};