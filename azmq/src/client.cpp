#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <random>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

using namespace boost;

struct generator {
  generator(int low, int high) : gen(rd()), dis(low, high) {}

  int next() {
    return dis(gen);
  }

  std::random_device rd;
  std::mt19937 gen;
  std::uniform_int_distribution<> dis;
};

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argc + argv);
  auto endpoint = args.size() > 0 ? args[0] : "tcp://127.0.0.1:9119";

  asio::io_service ios;
  asio::io_service::work work(ios);

  azmq::dealer_socket client(ios, true);
  generator gen(1, 1000);
  client.set_option(azmq::socket::identity("client:" + std::to_string(gen.next())));
  client.connect(endpoint);

  std::array<std::string, 3> messages = {
    "hello", "world!"
  };

  for (auto idx = 0; idx < 10; ++idx) {
    client.async_send(messages, [](const auto& ec, auto transferred) {

    });
  }

  ios.run();
  return 0;
}
