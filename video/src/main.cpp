#include <opencv2/opencv.hpp>
#include <iostream>

#include <ctime>
#include <cstdio>
#include <cstdlib>

using namespace cv;

void camCap() {
  VideoCapture cap(0); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return;

  Mat edges;
  namedWindow("edges",1);
  for(;;)
  {
    Mat frame;
    cap >> frame; // get a new frame from camera
    cvtColor(frame, edges, CV_8U);
    //cvtColor(frame, edges, CV_BGR2GRAY);
    GaussianBlur(edges, edges, Size(31,31), 8, 8);
    //Canny(edges, edges, 0, 30, 3);
    imshow("edges", edges);
    if(waitKey(30) >= 0) break;
  }
  // the camera will be deinitialized automatically in VideoCapture destructor
}

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

  stroke(unsigned int rows, unsigned int cols, unsigned int maxRadius) 
  : xp(randInt(rows)), yp(randInt(cols)), vx(0), vy(0), radius(randInt(maxRadius)) { }

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

std::vector<stroke> initStrokes(const unsigned int rows, const unsigned int cols, const unsigned int maxRadius, const unsigned int num) {
  std::vector<stroke> strokes;
  for (unsigned int x = 0; x < num; ++x) {
   strokes.emplace_back(rows, cols, maxRadius);
  }
  return strokes;
}

template <typename Image>
void blur2(Image& img) {
  static std::vector<stroke> strokes = initStrokes(img.rows, img.cols, 4, 40000);
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
void blur(Image& img) {
  const int iterations = 20000;
  const int maxRadius = 7;
  for (unsigned int i = 0; i < iterations; ++i) {
    int row = randInt(img.rows);
    int col = randInt(img.cols);
    int radius = randInt(maxRadius);

    int count = 0;
    int red = 0;
    int green = 0;
    int blue = 0;

    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if ((x * x + y * y) < (radius * radius)) {
          unsigned int xo = (row + x + img.rows) % img.rows;
          unsigned int yo = (col + y + img.cols) % img.cols;
          Vec3b& pixel = img.template at<Vec3b>(xo, yo);
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

    for (int x = -radius; x <= radius; ++x) {
      for (int y = -radius; y <= radius; ++y) {
        if ((x * x + y * y) < (radius * radius)) {
          unsigned int xo = (row + x + img.rows) % img.rows;
          unsigned int yo = (col + y + img.cols) % img.cols;
          Vec3b& pixel = img.template at<Vec3b>(xo, yo);
          pixel[0] = blue;
          pixel[1] = green;
          pixel[2] = red;
        }
      }
    }
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

void vidCap(const std::string& file) {
  //VideoCapture cap(file.c_str()); // open the default camera
  VideoCapture cap(0);
  if(!cap.isOpened())  // check if we succeeded
    return;

  Mat edges;
  namedWindow("edges",CV_WINDOW_KEEPRATIO);
  for(;;)
  {
    Mat frame;
    cap >> frame; // get a new frame from camera
    //GaussianBlur(edges, edges, Size(31,31), 3, 3);
      
    /*
    unsigned int colors = 6;
    blur2(frame);
    modChannel2(frame, 0, colors);
    modChannel2(frame, 1, colors);
    modChannel2(frame, 2, colors);
    for (int i = 0; i < 10; ++i)
      GaussianBlur(edges, edges, Size(15,15), 8, 8);
    */

    unsigned int colors = 6;
    blur2(frame);
    modChannel(frame, 0, colors);
    modChannel(frame, 1, colors);
    modChannel(frame, 2, colors);
    GaussianBlur(edges, edges, Size(7,7), 2, 2);

    //blur2(frame);
    //blur(frame);
    //clearChannel(frame, 0);
    imshow("edges", frame);
    //std::cout << (unsigned char) frame.at<Vec3b>(0,0)[0] << std::endl;
    //cvtColor(frame, edges, CV_8U);
    //cvtColor(frame, edges, CV_BGR2GRAY);
    //GaussianBlur(edges, edges, Size(31,31), 8, 8);
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
