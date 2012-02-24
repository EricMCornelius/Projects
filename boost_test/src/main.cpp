#include <boost/thread.hpp>

#include <iostream>
#include <vector>

//#include <redirect.hpp>

boost::mutex lock;
std::vector<unsigned int> output;

void helloWorld(unsigned int iterations) {
  for (unsigned int count = 0; count < iterations; ++count) {
    boost::lock_guard<boost::mutex> guard(lock);
    output.push_back(count);
  }
}

int main(int argc, char* argv[]) {
  //RedirectIOToConsole();
  std::vector<boost::thread> threadVec;
  unsigned int threads = 3;
  for (unsigned int i = 0; i < threads; ++i)
	threadVec.emplace_back(helloWorld, 50000);
	
  for (auto& t : threadVec)
	t.join();
	
  unsigned int last = 0;
  for (auto i : output) {
    if (last > i)
	  std::cout << last << " > " << i << std::endl;
	last = i;
  }
	
  //int x;
  //std::cin >> x;
}
