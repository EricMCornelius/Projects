#include <ring_buffer.hpp>

#include <iostream>
#include <vector>
#include <string>

int main (int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argc + argv);

  //ring_buffer buffer("test.dat", 10);
  ring_buffer_impl<1024> buffer("test.dat", 8);
  buffer.push((uint8_t*)"hello", 5);
  buffer.push((uint8_t*)"world", 5);
  uint8_t buf[4096] = {"hello world"};
  buffer.push(buf, 4096);
  buffer.print();

  auto elem = args.size() > 0 ? std::stoi(args[0]) : 1;
  auto res = buffer.get(elem);
  std::cout << res.string() << std::endl;
  return 0;
}
