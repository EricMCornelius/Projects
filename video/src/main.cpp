#include <opencv2/opencv.hpp>
#include <iostream>

#include <ctime>
#include <cstdio>
#include <cstdlib>

using namespace cv;

unsigned int randInt(int max) {
  return (std::rand() % max);
}

int randInt(int min, int max) {
  return (std::rand() % (max - min + 1)) + min;
}

struct stroke {
  int xp;
  int yp;
  int vx;
  int vy;
  int radius;

  stroke(unsigned int rows, unsigned int cols, unsigned int minRadius, unsigned int maxRadius) 
  : xp(randInt(rows)), yp(randInt(cols)), vx(0), vy(0), radius(randInt(minRadius, maxRadius)) { }

  template <typename Image>
  void getAvg(int& red, int& green, int& blue, Image& img) {
    int count = 0;

    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if ((x * x + y * y) < (radius * radius)) {
          unsigned int xo = (xp + x + img.rows) % img.rows;
          unsigned int yo = (yp + y + img.cols) % img.cols;
          const Vec3b& pixel = img.template at<Vec3b>(xo, yo);
          blue += pixel[0];
          green += pixel[1];
          red += pixel[2];
          ++count;
        }
      }
    }

    if (count != 0) {
      blue /= count;
      green /= count;
      red /= count;
    }
  }

  template <typename Image>
  void setAvg(unsigned int red, unsigned int green, unsigned int blue, Image& img) {
    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if ((x * x + y * y) < (radius * radius)) {
          unsigned int xo = (xp + x + img.rows) % img.rows;
          unsigned int yo = (yp + y + img.cols) % img.cols;
          Vec3b& pixel = img.template at<Vec3b>(xo, yo);
          pixel[0] = blue;
          pixel[1] = green;
          pixel[2] = red;
        }
      }
    }
  }

  void shift(unsigned int rows, unsigned int cols) {
    vx += randInt(-1,1);
    vy += randInt(-1,1);
    xp += (vx + rows) % rows;
    yp += (vy + cols) % cols;
  }
};

std::vector<stroke> initStrokes(const unsigned int rows, const unsigned int cols, const unsigned int minRadius, const unsigned int maxRadius, const unsigned int num) {
  std::vector<stroke> strokes;
  for (unsigned int x = 0; x < num; ++x) {
    strokes.emplace_back(rows, cols, minRadius, maxRadius);
  }
  return strokes;
}

template <typename Image>
void blur(Image& img) {
  static std::vector<stroke> strokes = initStrokes(img.rows, img.cols, 1, 4, 40000);
  for (stroke& obj : strokes) {
    int red = 0;
    int green = 0;
    int blue = 0;

    obj.getAvg(red, green, blue, img);
    obj.setAvg(red, green, blue, img);
    if (randInt(25) == 0)
      obj.shift(img.rows, img.cols);
  }
}

template <typename Image>
void clearChannel(Image& img, unsigned int channel) {
  MatIterator_<Vec3b> itr = img.template begin<Vec3b>();
  while (itr != img.template end<Vec3b>()) {
    Vec3b& pixel = *itr;
    pixel[channel] = 0;
    ++itr;
  }
}

template <typename Image>
void modChannel(Image& img, unsigned int channel, unsigned int colors) {
  unsigned int range = 256 / colors;
  MatIterator_<Vec3b> itr = img.template begin<Vec3b>();
  while (itr != img.template end<Vec3b>()) {
    Vec3b& pixel = *itr;
    pixel[channel] -= (pixel[channel] % range);
    ++itr;
  }
}

template <typename Image>
void modChannel2(Image& img, unsigned int channel, unsigned int colors) {
  unsigned int range = 256 / colors;
  MatIterator_<Vec3b> itr = img.template begin<Vec3b>();
  while (itr != img.template end<Vec3b>()) {
    Vec3b& pixel = *itr;
    pixel[channel] -= (pixel[channel] % range);
    ++itr;
  }
}

void renderLoop(VideoCapture& cap);

void vidCap(const std::string& file) {
  VideoCapture cap(file.c_str());
  if (!cap.isOpened())
    return;

  renderLoop(cap);
}

void vidCap(const int deviceId) {
  VideoCapture cap(deviceId);
  if (!cap.isOpened())
    return;

  renderLoop(cap);
}


void renderLoop(VideoCapture& cap) {
  Mat edges;
  namedWindow("edges",CV_WINDOW_KEEPRATIO);
  for(;;)
  {
    Mat frame;
    cap >> frame; // get a new frame from camera

    unsigned int colors = 6;
    modChannel(frame, 0, colors);
    modChannel(frame, 1, colors);
    modChannel(frame, 2, colors);
    GaussianBlur(edges, edges, Size(7,7), 2, 2);
    blur(frame);

    imshow("edges", frame);
    //Canny(edges, edges, 0, 30, 3);
    //imshow("edges", edges);
    if(waitKey(30) >= 0) break;
  }
  // the camera will be deinitialized automatically in VideoCapture destructor
}

int main(int, char**)
{
  std::srand(std::time(0));
  vidCap("test.avi");
  int x;
  std::cin >> x;
  return 0;
}
