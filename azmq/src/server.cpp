#include <iostream>
#include <string>
#include <vector>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <array>

using namespace boost;

template <typename Socket, typename Callback>
void receive(Socket& s, Callback& cb) {
  s.async_receive([&](auto& ec, azmq::message& msg, std::size_t size) {
    std::vector<azmq::message> results;
    results.emplace_back(std::move(msg));

    if (results[0].more()) {
      azmq::message_vector vec;
      s.receive_more(vec, 0, ec);
      for (const auto& msg : vec) {
        results.emplace_back(std::move(msg));
      }
    }

    cb(s, std::move(results));
    receive(s, cb);
  });
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv + 1, argc + argv);
  auto endpoint = args.size() > 0 ? args[0] : "tcp://0.0.0.0:9119";

  asio::io_service ios;

  auto handler = [](auto& socket, std::vector<azmq::message>&& parts) {
    for (auto& part : parts) {
      std::cout << part.string() << " ";
    }
    std::cout << std::endl;
  };

  auto num_sockets = 3;
  std::vector<azmq::socket> sockets;
  for (auto idx = 0u; idx < num_sockets; ++idx) {
    azmq::router_socket server(ios, true);
    server.bind(endpoint);
    std::cout << server.endpoint() << std::endl;
    sockets.emplace_back(std::move(server));
  }

  for (auto& socket : sockets) {
    receive(socket, handler);
  }

  ios.run();
  return 0;
}
