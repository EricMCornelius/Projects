#define BOOST_CB_DISABLE_DEBUG 1

#include <vector>
#include <string>

#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>

#include <boost/circular_buffer.hpp>

using namespace boost;
using namespace boost::interprocess;

typedef allocator<char, managed_mapped_file::segment_manager> char_allocator;
typedef basic_string<char, std::char_traits<char>, char_allocator> mapped_string;
typedef allocator<mapped_string, managed_mapped_file::segment_manager> string_allocator;
typedef circular_buffer<mapped_string, string_allocator> mapped_buffer;

int main (int argc, char* argv[]) {
  auto filename = "test.dat";
  auto num_elems = 1024;
  auto segment_size = (1024 * 1024 / mapped_region::get_page_size() + 1) * mapped_region::get_page_size();
  managed_mapped_file file(open_or_create, filename, segment_size);
  const char_allocator char_alloc(file.get_segment_manager());
  const string_allocator string_alloc(file.get_segment_manager());

  try {

  mapped_buffer* bufPtr = file.find_or_construct<mapped_buffer>("my_buffer")(num_elems, string_alloc);
  auto& buf = *bufPtr;
  auto offset = 0;
  if (buf.size() > 0) {
    auto last = buf.back();
    offset = std::stoi(last.substr(6).c_str());
  }

  for (auto idx = 1; idx <= 128; ++idx) {
    mapped_string val(("elem: " + std::to_string(idx + offset)).c_str(), char_alloc);
    buf.push_back(std::move(val));
  }

  std::cout << buf.size() << std::endl;
  for (auto& val : buf) {
    std::cout << val << std::endl;
  }

  }
  catch(std::exception& e) {
    std::cout << e.what() << std::endl;
    managed_mapped_file::grow(filename, 1024);
  }

  return 0;
}
