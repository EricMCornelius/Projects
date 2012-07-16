#include <test.h>
#include <iostream>

template <typename T>
void print(const T& t) {
  std::cout << t << std::endl;
}

template <typename Head, typename... Tail>
void print(const Head& head, const Tail&... tail) {
  std::cout << head << " ";
  print(tail...);
}

void print() {
  print("This", "is", "test", "number: ", 2);
}
