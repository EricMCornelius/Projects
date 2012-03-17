#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <memory>
#include <mutex>
#include <future>

std::mutex lock;

void print(const std::string& msg) {
  std::lock_guard<std::mutex> guard(lock);
  std::cout << msg << std::endl;
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args(argv, argc + argv);
  typedef std::future<void> FutureType;
  std::vector<FutureType> futures;
  for (auto& arg : args)
    futures.emplace_back(std::async(std::launch::async, print, arg));
    
  for (auto& future : futures)
    future.wait();
}