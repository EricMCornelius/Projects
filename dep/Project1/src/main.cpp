#include <iostream>
#include <test.h>

#include <boost/thread.hpp>

int main(int argc, char* argv[]) {
  std::cout << "Hello World" << std::endl;
  boost::thread t1(print);
  t1.join();
}
