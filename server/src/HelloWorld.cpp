//============================================================================
// Name        : HelloWorld.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>

#include <Server.h>
#include <BlockingQueue.h>

#include <thread>
#include <iostream>

void test(size_t val) {
  val += 1;
  const int x = 32 - __builtin_clz(val) - 1;
  const size_t y = (1 << x) ^ val;

  std::cout << x << " " << y << std::endl;
}

int main() {

  /*
  boost::filesystem3::path p = boost::filesystem3::current_path();
  std::cout << p << std::endl;

  for (auto& path : p)
    std::cout << path << std::endl;

  boost::filesystem3::directory_iterator itr(p);
  boost::filesystem3::directory_iterator end;
  for (; itr != end; ++itr)
    std::cout << (*itr).path() << std::endl;
  */
    
  boost::asio::io_service loop;
  Server server(loop);
  server.listen(9091);
  loop.run();

  //std::cout << __builtin_ffs(val) << std::endl;
  //std::cout << __builtin_ctz(val) << std::endl;

  //for (size_t val = 0; val < 65535; ++val)
  //  test(val);

  return 0;
}
