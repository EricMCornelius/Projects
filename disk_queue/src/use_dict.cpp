#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <lz4.h>

int main(int argc, char* argv[]) {
  std::ifstream input(argv[1]);
  std::stringstream str;
  str << input.rdbuf();
  auto text = str.str();

  std::ifstream dictionary(argv[2]);
  std::stringstream str2;
  str2 << dictionary.rdbuf();
  auto dict = str2.str();

  std::ofstream output(argv[3]);
  std::array<char, 16384> buffer;
  auto res = LZ4_decompress_safe_usingDict(&text[0], &buffer[0], text.size(), buffer.size(), &dict[0], dict.size());
  output.write(&buffer[0], res);
}
