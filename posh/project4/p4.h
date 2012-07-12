#ifndef P4_H
#define P4_H

#include <iostream>

template <typename T>
void print(const T& obj) {
  std::cout << obj << std::endl;
}

template <typename T, typename... Tail>
void print(const T& obj, const Tail&... tail) {
  std::cout << obj << " ";
  print(tail...);
}

void p4() {
  print("Booyah!", "Cool!");
}

void t1() { }

#endif
