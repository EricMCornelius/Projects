#include <Magick++.h>
#include <string>
#include <array>
#include <cmath>

#include <iostream>

template <typename Color>
Color magnitude(Color& c1, Color& c2) {
  return Color(std::sqrt(c1.red() * c1.red() + c2.red() * c2.red()),
               std::sqrt(c1.green() * c1.green() + c2.green() * c2.green()),
               std::sqrt(c1.blue() * c1.blue() + c2.blue() * c2.blue()));
}

int main(int argc, char* argv[]) {
  using namespace Magick;

  InitializeMagick(*argv);

  std::string filename;
  std::cout << "File: ";
  std::getline(std::cin, filename);

  Image im(filename);

  std::array<double, 9> kernel_x = {-1, 0, 1, 
                                    -2, 0, 2, 
                                    -1, 0, 1};
  std::array<double, 9> kernel_y = {-1, -2, -1, 
                                     0,  0,  0,
                                     1,  2,  1};

  Image dx(im);
  dx.convolve(3, kernel_x.data());

  Image dy(im);
  dy.convolve(3, kernel_y.data());

  Geometry geo(im.columns(), im.rows());
  Image red(geo, ColorGray());
  Image green(geo, ColorGray());
  Image blue(geo, ColorGray());

  for (int y = 0; y < im.rows(); ++y) {
    for (int x = 0; x < im.columns(); ++x) {
      auto dx_color = ColorRGB(dx.pixelColor(x, y));
      auto dy_color = ColorRGB(dy.pixelColor(x, y));
      auto m = magnitude(dx_color, dy_color);
      red.pixelColor(x, y, ColorGray(m.red()));
      green.pixelColor(x, y, ColorGray(m.green()));
      blue.pixelColor(x, y, ColorGray(m.blue()));
    }
  }

  im.normalize();
  im.display();
  red.display();
  green.display();
  blue.display();
  //im.display();
  // im.write("output.tiff");
}
