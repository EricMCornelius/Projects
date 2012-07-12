#include <test.h>
#include <cool.h>
#include <iostream>

#include <boost/thread.hpp>

void functor() {
  print();
  cool();
}

int main(int argc, char* argv[]) {
  boost::thread t(functor);
  t.join();
}
