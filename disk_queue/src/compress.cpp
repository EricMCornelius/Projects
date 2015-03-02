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

  auto stream = LZ4_createStream();
  std::array<char, 64 * 1024> output_buffer;
  LZ4_loadDict(stream, &dict[0], dict.size());
  auto res = LZ4_compress_limitedOutput_continue(stream, &text[0], &output_buffer[0], text.size(), output_buffer.size());
  output.write(&output_buffer[0], res);
}
