#pragma once

#include <sstream>
#include <string>

std::string format(std::stringstream& out) { 
  return out.str();
}

template <typename Head, typename... Tail>
std::string format(std::stringstream& out, const Head& head, const Tail&... tail) {
  out << head;
  return format(out, tail...);
}

template <typename... Args>
std::string format(const Args&... args) {
  std::stringstream stream;
  return format(stream, args...);
}