#include <ring_buffer.hpp>
#include <persistent_cursor.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

int main (int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argc + argv);

  std::cout << "initializing buffers..." << std::endl;
  constexpr auto message_size = 256;
  ring_buffer_impl<message_size> buffer("test.dat", 1000000);
  std::cout << "buffers initialized" << std::endl;
  std::cout << "inserting data" << std::endl;
  for (auto idx = 0; idx < 1000000; ++idx) {
    uint8_t buf[message_size] = {"message: "};
    auto str = std::to_string(idx);
    std::copy(std::begin(str), std::end(str), buf + 8);
    buffer.push(buf, message_size);
  }
  std::cout << "data inserted" << std::endl;
  // buffer.print();

  auto& cursor = persistent_cursor("consumer_offset").get();
  std::cout << cursor << std::endl;

  auto elem = args.size() > 0 ? std::stoi(args[0]) : 1;
  auto res = buffer.get(elem);
  std::cout << res.string() << std::endl;

  cursor = elem;
  return 0;
}
