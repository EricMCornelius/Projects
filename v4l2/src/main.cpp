#include <iostream>

#include <v4l2.hpp>

int main(int argc, char* argv[]) {
  auto devices = getDevices();
  for (auto& d : devices) {
    std::cout << "name: " << d.name
              << "\nbus: " << d.bus
              << "\nvideo capture: " << d.supports(V4L2_CAP_VIDEO_CAPTURE)
              << "\naudio capture: " << d.supports(V4L2_CAP_AUDIO) << std::endl;

    for (auto& f : d.formats) {
      std::cout << "format: " << f.description << " - " << f.width << ":" << f.height << std::endl;
    }
  }
}