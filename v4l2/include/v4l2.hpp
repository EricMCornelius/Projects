#pragma once

#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstring>

#include <linux/videodev2.h>
#include <libv4l2.h>

#include <vector>

#include <device.hpp>
#include <format.hpp>

const std::vector<std::pair<int, std::string>> formats = { 
{ V4L2_PIX_FMT_RGB332, "RGB332" },
{ V4L2_PIX_FMT_RGB444, "RGB444" },
{ V4L2_PIX_FMT_RGB555, "RGB555" },
{ V4L2_PIX_FMT_RGB565, "RGB565" },
{ V4L2_PIX_FMT_RGB555X, "RGB555X" },
{ V4L2_PIX_FMT_RGB565X, "RGB565X" },
{ V4L2_PIX_FMT_BGR666, "BGR666" },
{ V4L2_PIX_FMT_BGR24, "BGR24" },
{ V4L2_PIX_FMT_RGB24, "RGB24" },
{ V4L2_PIX_FMT_BGR32, "BGR32" },
{ V4L2_PIX_FMT_RGB32, "RGB32" },
{ V4L2_PIX_FMT_GREY, "GREY" },
{ V4L2_PIX_FMT_Y4, "Y4" },
{ V4L2_PIX_FMT_Y6, "Y6" },
{ V4L2_PIX_FMT_Y10, "Y10" },
{ V4L2_PIX_FMT_Y12, "Y12" },
{ V4L2_PIX_FMT_Y16, "Y16" },
{ V4L2_PIX_FMT_Y10BPACK, "Y10BPACK" },
{ V4L2_PIX_FMT_PAL8, "PAL8" },
{ V4L2_PIX_FMT_YVU410, "YVU410" },
{ V4L2_PIX_FMT_YVU420, "YVU420" },
{ V4L2_PIX_FMT_YUYV, "YUYV" },
{ V4L2_PIX_FMT_YYUV, "YYUV" },
{ V4L2_PIX_FMT_YVYU, "YVYU" },
{ V4L2_PIX_FMT_UYVY, "UYVY" },
{ V4L2_PIX_FMT_VYUY, "VYUY" },
{ V4L2_PIX_FMT_YUV422P, "YUV422P" },
{ V4L2_PIX_FMT_YUV411P, "YUV411P" },
{ V4L2_PIX_FMT_Y41P, "Y41P" },
{ V4L2_PIX_FMT_YUV444, "YUV444" },
{ V4L2_PIX_FMT_YUV555, "YUV555" },
{ V4L2_PIX_FMT_YUV565, "YUV565" },
{ V4L2_PIX_FMT_YUV32, "YUV32" },
{ V4L2_PIX_FMT_YUV410, "YUV410" },
{ V4L2_PIX_FMT_YUV420, "YUV420" },
{ V4L2_PIX_FMT_HI240, "HI240" },
{ V4L2_PIX_FMT_HM12, "HM12" },
{ V4L2_PIX_FMT_M420, "M420" },
{ V4L2_PIX_FMT_NV12, "NV12" },
{ V4L2_PIX_FMT_NV21, "NV21" },
{ V4L2_PIX_FMT_NV16, "NV16" },
{ V4L2_PIX_FMT_NV61, "NV61" },
{ V4L2_PIX_FMT_NV24, "NV24" },
{ V4L2_PIX_FMT_NV42, "NV42" },
{ V4L2_PIX_FMT_NV12M, "NV12M" },
//{ V4L2_PIX_FMT_NV21M, "NV21M" },
{ V4L2_PIX_FMT_NV12MT, "NV12MT" },
//{ V4L2_PIX_FMT_NV12MT_16X16, "NV12MT_16X16" },
{ V4L2_PIX_FMT_YUV420M, "YUV420M" },
//{ V4L2_PIX_FMT_YVU420M, "YVU420M" },
{ V4L2_PIX_FMT_SBGGR8, "SBGGR8" },
{ V4L2_PIX_FMT_SGBRG8, "SGBRG8" },
{ V4L2_PIX_FMT_SGRBG8, "SGRBG8" },
{ V4L2_PIX_FMT_SRGGB8, "SRGGB8" },
{ V4L2_PIX_FMT_SBGGR10, "SBGGR10" },
{ V4L2_PIX_FMT_SGBRG10, "SGBRG10" },
{ V4L2_PIX_FMT_SGRBG10, "SGRBG10" },
{ V4L2_PIX_FMT_SRGGB10, "SRGGB10" },
{ V4L2_PIX_FMT_SBGGR12, "SBGGR12" },
{ V4L2_PIX_FMT_SGBRG12, "SGBRG12" },
{ V4L2_PIX_FMT_SGRBG12, "SGRBG12" },
{ V4L2_PIX_FMT_SRGGB12, "SRGGB12" },
{ V4L2_PIX_FMT_SBGGR10DPCM8, "SBGGR10DPCM8" },
{ V4L2_PIX_FMT_SGBRG10DPCM8, "SGBRG10DPCM8" },
{ V4L2_PIX_FMT_SGRBG10DPCM8, "SGRBG10DPCM8" },
{ V4L2_PIX_FMT_SRGGB10DPCM8, "SRGGB10DPCM8" },
{ V4L2_PIX_FMT_SBGGR16, "SBGGR16" },
{ V4L2_PIX_FMT_MJPEG, "MJPEG" },
{ V4L2_PIX_FMT_JPEG, "JPEG" },
{ V4L2_PIX_FMT_DV, "DV" },
{ V4L2_PIX_FMT_MPEG, "MPEG" },
{ V4L2_PIX_FMT_H264, "H264" },
{ V4L2_PIX_FMT_H264_NO_SC, "H264_NO_SC" },
//{ V4L2_PIX_FMT_H264_MVC, "H264_MVC" },
{ V4L2_PIX_FMT_H263, "H263" },
{ V4L2_PIX_FMT_MPEG1, "MPEG1" },
{ V4L2_PIX_FMT_MPEG2, "MPEG2" },
{ V4L2_PIX_FMT_MPEG4, "MPEG4" },
{ V4L2_PIX_FMT_XVID, "XVID" },
{ V4L2_PIX_FMT_VC1_ANNEX_G, "VC1_ANNEX_G" },
{ V4L2_PIX_FMT_VC1_ANNEX_L, "VC1_ANNEX_L" },
//{ V4L2_PIX_FMT_VP8, "VP8" },
{ V4L2_PIX_FMT_CPIA1, "CPIA1" },
{ V4L2_PIX_FMT_WNVA, "WNVA" },
{ V4L2_PIX_FMT_SN9C10X, "SN9C10X" },
{ V4L2_PIX_FMT_SN9C20X_I420, "SN9C20X_I420" },
{ V4L2_PIX_FMT_PWC1, "PWC1" },
{ V4L2_PIX_FMT_PWC2, "PWC2" },
{ V4L2_PIX_FMT_ET61X251, "ET61X251" },
{ V4L2_PIX_FMT_SPCA501, "SPCA501" },
{ V4L2_PIX_FMT_SPCA505, "SPCA505" },
{ V4L2_PIX_FMT_SPCA508, "SPCA508" },
{ V4L2_PIX_FMT_SPCA561, "SPCA561" },
{ V4L2_PIX_FMT_PAC207, "PAC207" },
{ V4L2_PIX_FMT_MR97310A, "MR97310A" },
{ V4L2_PIX_FMT_JL2005BCD, "JL2005BCD" },
{ V4L2_PIX_FMT_SN9C2028, "SN9C2028" },
{ V4L2_PIX_FMT_SQ905C, "SQ905C" },
{ V4L2_PIX_FMT_PJPG, "PJPG" },
{ V4L2_PIX_FMT_OV511, "OV511" },
{ V4L2_PIX_FMT_OV518, "OV518" },
{ V4L2_PIX_FMT_STV0680, "STV0680" },
{ V4L2_PIX_FMT_TM6000, "TM6000" },
{ V4L2_PIX_FMT_CIT_YYVYUY, "CIT_YYVYUY" },
{ V4L2_PIX_FMT_KONICA420, "KONICA420" },
{ V4L2_PIX_FMT_JPGL, "JPGL" },
{ V4L2_PIX_FMT_SE401, "SE401" },
//{ V4L2_PIX_FMT_S5C_UYVY_JPG, "S5C_UYVY_JPG" }
};

const std::vector<std::pair<int, int>> resolutions = {
  { 128, 96 }, { 160, 120 }, { 176, 144 },
  { 320, 240 }, { 352, 288 }, { 640, 480 },
  { 704, 576 }, { 800, 600 }, { 960, 720 },
  { 1280, 720 }, { 1024, 768 }, { 1440, 1080 },
  { 1920, 1080 } 
};

template <typename Type>
int xioctl(int fh, int request, Type *arg)
{
  int r;

  do {
    r = v4l2_ioctl(fh, request, arg);
  } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

  return r;
}


Device& fillFormats(Device& d) {
  struct v4l2_format video_fmt;
  std::memset(&video_fmt, 0, sizeof(struct v4l2_format));

  video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  //video_fmt.fmt.pix.sizeimage = 0;
  video_fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  for (const auto& format : formats) {
    for (const auto& resolution : resolutions) {
      video_fmt.fmt.pix.pixelformat = format.first;
      video_fmt.fmt.pix.width = resolution.first;
      video_fmt.fmt.pix.height = resolution.second;

      if (xioctl(d.handle, VIDIOC_TRY_FMT, &video_fmt) >= 0) {
        if (video_fmt.fmt.pix.width == resolution.first && video_fmt.fmt.pix.height == resolution.second) {
          Format f;
          f.width = video_fmt.fmt.pix.width;
          f.height = video_fmt.fmt.pix.height;
          f.description = format.second;

          d.formats.push_back(f);
        }
      }
    }
  }

  return d;
}

std::vector<Device> getDevices() {
  std::vector<Device> devices;

  for (auto i = 0; i < 64; ++i) {
    Device d;
    d.id = format("/dev/video", i);
    d.handle = v4l2_open(d.id.c_str(), O_RDONLY);
    if (d.handle == -1)
      break;

    struct v4l2_capability cap;
    if (xioctl(d.handle, VIDIOC_QUERYCAP, &cap) < 0) {
      v4l2_close(d.handle);
      break;
    }

    d.name = reinterpret_cast<const char*>(cap.card);
    d.bus = reinterpret_cast<const char*>(cap.bus_info);
    d.capabilities = cap.capabilities;

    fillFormats(d);
    v4l2_close(d.handle);

    devices.push_back(d);
  }

  return devices;
}
