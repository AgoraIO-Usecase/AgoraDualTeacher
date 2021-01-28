#ifndef AGORA_VIDEO_SOURCE_ENGINE_H
#define AGORA_VIDEO_SOURCE_ENGINE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define AGORA_CALL __cdecl
#if defined(AGORARTC_EXPORT)
#define AGORA_API extern "C" __declspec(dllexport)
#define AGORA_CPP_API __declspec(dllexport)
#else
#define AGORA_API extern "C" __declspec(dllimport)
#define AGORA_CPP_API __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#define AGORA_API __attribute__((visibility("default"))) extern "C"
#define AGORA_CPP_API __attribute__((visibility("default")))
#define AGORA_CALL
#elif defined(__ANDROID__) || defined(__linux__)
#define AGORA_API extern "C" __attribute__((visibility("default")))
#define AGORA_CPP_API __attribute__((visibility("default")))
#define AGORA_CALL
#else
#define AGORA_API extern "C"
#define AGORA_CPP_API
#define AGORA_CALL
#endif

#include <functional>
#include <memory>

namespace agora {

struct image_frame_info {
  void* view;
  int stride;
  int stride0;
  int width;
  int height;
  int strideU;
  int strideV;
};

struct image_header_type {
  unsigned char format;
  unsigned char mirrored;
  unsigned short width;
  unsigned short height;
  unsigned short left;
  unsigned short top;
  unsigned short right;
  unsigned short bottom;
  unsigned short rotation;
  unsigned int timestamp;
};

class agora_video_frame {
 public:
  enum PLANE_TYPE_ { Y_PLANE = 0, U_PLANE = 1, V_PLANE = 2, NUM_OF_PLANES = 3 };
  enum VIDEO_TYPE_ {
    VIDEO_TYPE_UNKNOWN = 0,
    VIDEO_TYPE_I420 = 1,
    VIDEO_TYPE_IYUV = 2,
    VIDEO_TYPE_RGB24 = 3,
    VIDEO_TYPE_ABGR = 4,
    VIDEO_TYPE_ARGB = 5,
    VIDEO_TYPE_ARGB4444 = 6,
    VIDEO_TYPE_RGB565 = 7,
    VIDEO_TYPE_ARGB1555 = 8,
    VIDEO_TYPE_YUY2 = 9,
    VIDEO_TYPE_YV12 = 10,
    VIDEO_TYPE_UYVY = 11,
    VIDEO_TYPE_MJPG = 12,
    VIDEO_TYPE_NV21 = 13,
    VIDEO_TYPE_NV12 = 14,
    VIDEO_TYPE_BGRA = 15,
    VIDEO_TYPE_RGBA = 16,
    VIDEO_TYPE_I422 = 17,
  };

 public:
  agora_video_frame() {}
  agora_video_frame(const image_header_type& header, unsigned short width,
                    unsigned short height, unsigned char* stride_y,
                    int stride_y_size, unsigned char* stride_u,
                    int stride_u_size, unsigned char* stride_v,
                    int sride_v_size, unsigned int timestamp)
      : width_(width),
        height_(height),
        stride_y_size_(stride_y_size),
        stride_u_size_(stride_u_size),
        stride_v_size_(sride_v_size),
        timestamp_(timestamp),
        header_(header)

  {
    int size = stride_y_size_ * 1.5 * height_;
    auto buf = new unsigned char[size];
    std::memcpy(buf, stride_y, size);
    buffer_.reset(buf);
  }

  image_header_type get_image_header_() { return header_; }

  agora_video_frame(const agora_video_frame& lrs)
      : width_(lrs.width_),
        height_(lrs.height_),
        stride_y_size_(lrs.stride_y_size_),
        stride_u_size_(lrs.stride_u_size_),
        stride_v_size_(lrs.stride_v_size_),
        timestamp_(lrs.timestamp_) {
    header_ = lrs.header_;
    buffer_ = lrs.buffer_;
  }
  virtual VIDEO_TYPE_ GetVideoType() const { return VIDEO_TYPE_YV12; }
  virtual void release() { buffer_.reset(); }
  /*!
   * @brief
   * @author LuCheng
   * @date   2020/11/5
   * @param  PLANE_TYPE type
   * @return     const unsigned char*
   */
  virtual const unsigned char* buffer(PLANE_TYPE_ type) const {
    switch (type) {
      case Y_PLANE:
        return buffer_.get();
      case U_PLANE:
        return buffer_.get() + allocated_size(Y_PLANE);
      case V_PLANE:
        return buffer_.get() + allocated_size(Y_PLANE) +
               allocated_size(U_PLANE);
    }
    return nullptr;
  }

  // Copy frame: If required size is bigger than allocated one, new buffers of
  // adequate size will be allocated.
  // Return value: 0 on success ,-1 on error.
  virtual int copyFrame(agora_video_frame** dest_frame) const {
    if (!dest_frame) return -1;
    if (!*dest_frame) {
      *dest_frame = new agora_video_frame(
          header_, width_, height_, (unsigned char*)buffer(Y_PLANE),
          stride_y_size_, (unsigned char*)buffer(U_PLANE), stride_u_size_,
          (unsigned char*)buffer(V_PLANE), stride_v_size_, timestamp_);
      return 0;
    }

    agora_video_frame* video_frame =
        dynamic_cast<agora_video_frame*>(*dest_frame);
    if (video_frame->allocated_size(agora_video_frame::Y_PLANE) >
        this->allocated_size(agora_video_frame::Y_PLANE)) {
      video_frame->release();
      video_frame = new agora_video_frame(*this);
      return 0;
    }
    video_frame->stride_y_size_ = stride_y_size_;
    video_frame->stride_u_size_ = stride_u_size_;
    video_frame->stride_v_size_ = stride_v_size_;
    int size = stride_y_size_ * 1.5 * height_;
    auto buf = new unsigned char[size];
    std::memcpy(buf, buffer_.get(), size);
    video_frame->buffer_.reset(buf);
    return 0;
  }

  // Convert frame
  // Input:
  //   - src_frame        : Reference to a source frame.
  //   - dst_video_type   : Type of output video.
  //   - dst_sample_size  : Required only for the parsing of MJPG.
  //   - dst_frame        : Pointer to a destination frame.
  // Return value: 0 if OK, < 0 otherwise.
  // It is assumed that source and destination have equal height.
  virtual int convertFrame(VIDEO_TYPE_ dst_video_type, int dst_sample_size,
                           unsigned char* dst_frame) const {
    return -1;
  }

  // Get allocated size per plane.
  virtual int allocated_size(PLANE_TYPE_ type) const {
    switch (type) {
      case agora_video_frame::Y_PLANE:
        return stride_y_size_ * height_;
      case agora_video_frame::U_PLANE:
        return stride_u_size_ * height_ / 2;
      case agora_video_frame::V_PLANE:
        return stride_v_size_ * height_ / 2;
    }
    return height_ * width_ * 1.5;
  }

  // Get allocated stride per plane.
  virtual int stride(PLANE_TYPE_ type) const {
    switch (type) {
      case agora_video_frame::Y_PLANE:
        return stride_y_size_;
      case agora_video_frame::U_PLANE:
        return stride_u_size_;
      case agora_video_frame::V_PLANE:
        return stride_v_size_;
    }
    return stride_y_size_;
  }

  // Get frame width.
  virtual int width() const { return width_; }

  // Get frame height.
  virtual int height() const { return height_; }
  // Get frame timestamp (90kHz).
  virtual unsigned int timestamp() const { return 0; }

  // Get render time in milliseconds.
  virtual int64_t render_time_ms() const { return timestamp_; }

  // Return true if underlying plane buffers are of zero size, false if not.
  virtual bool IsZeroSize() const { return !buffer_.get(); }

  virtual int getRotation() const { return header_.rotation; }
  virtual bool getMirror() const { return header_.mirrored; }

 private:
  image_header_type header_;
  int width_;
  int height_;
  std::shared_ptr<unsigned char> buffer_;
  int stride_y_size_;
  int stride_u_size_;
  int stride_v_size_;
  int timestamp_;
};

/*!
 * \class parser_video_frame
 *
 * \ingroup GroupName
 *
 * \brief
 *
 * TODO: long description
 *
 * \note
 *
 * \author LC
 *
 * \version 1.0
 *
 * \date Ê®Ò»ÔÂ 2020
 *
 * Contact: lucheng@agora.io
 *
 */
class parser_video_frame {
 public:
  std::shared_ptr<agora_video_frame> gen_video_frame(unsigned char* buf,
                                                     int len, void** view) {
    if (!buf) return std::make_shared<agora_video_frame>();
    image_frame_info* info = reinterpret_cast<image_frame_info*>(buf);
    image_header_type* header =
        reinterpret_cast<image_header_type*>(buf + sizeof(image_frame_info));
    unsigned char* y_stride =
        buf + sizeof(image_frame_info) + sizeof(image_header_type);
    *view = info->view;
    return std::make_shared<agora_video_frame>(
        *header, header->width, header->height, y_stride, info->stride0,
        y_stride + info->stride0, info->strideU,
        y_stride + info->stride0 + info->strideU, info->strideV,
        header->timestamp);
  }
};

namespace rtc {

typedef unsigned int uid_t_;

using delegate_render = std::function<void(agora_video_frame*)>;

class IVideoSourceEventHandler {
 public:
  virtual ~IVideoSourceEventHandler() {}
  /**
   * Video source joined channel success event.
   * @param uid : video source's uid.
   */
  virtual void onVideoSourceJoinedChannelSuccess(uid_t_ uid) = 0;

  /**
   * Video source request new token event.
   */
  virtual void onVideoSourceRequestNewToken() = 0;

  /**
   * Video source leaved channel event.
   */
  virtual void onVideoSourceLeaveChannel() = 0;

  virtual void onVideoSourceExit() = 0;

  virtual void onVideoSourceFirstVideoFrame(int width, int height,
                                            int elapsed) = 0;
};

enum CHANNEL_PROFILE_TYPE_ {
  /** (Default) Communication. This profile applies to scenarios such as an
   * audio call or video call, where all users can publish and subscribe to
   * streams.
   */
  CHANNEL_PROFILE_COMMUNICATION_ = 0,
  /** Live streaming. In this profile, uses have roles, namely, host and
   * audience (default). A host both publishes and subscribes to streams, while
   * an audience subscribes to streams only. This profile applies to scenarios
   * such as a chat room or interactive video streaming.
   */
  CHANNEL_PROFILE_LIVE_BROADCASTING_ = 1,
  /** 2: Gaming. This profile uses a codec with a lower bitrate and consumes
   * less power. Applies to the gaming scenario, where all game players can talk
   * freely.
   */
  CHANNEL_PROFILE_GAME_ = 2,
};

enum VIDEO_PROFILE_TYPE_ {
  /** 0: 160 * 120, frame rate 15 fps, bitrate 65 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_120P_ = 0,
  /** 2: 120 * 120, frame rate 15 fps, bitrate 50 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_120P_3_ = 2,
  /** 10: 320*180, frame rate 15 fps, bitrate 140 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_180P_ = 10,
  /** 12: 180 * 180, frame rate 15 fps, bitrate 100 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_180P_3_ = 12,
  /** 13: 240 * 180, frame rate 15 fps, bitrate 120 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_180P_4_ = 13,
  /** 20: 320 * 240, frame rate 15 fps, bitrate 200 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_240P_ = 20,
  /** 22: 240 * 240, frame rate 15 fps, bitrate 140 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_240P_3_ = 22,
  /** 23: 424 * 240, frame rate 15 fps, bitrate 220 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_240P_4_ = 23,
  /** 30: 640 * 360, frame rate 15 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_ = 30,
  /** 32: 360 * 360, frame rate 15 fps, bitrate 260 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_3_ = 32,
  /** 33: 640 * 360, frame rate 30 fps, bitrate 600 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_4_ = 33,
  /** 35: 360 * 360, frame rate 30 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_6_ = 35,
  /** 36: 480 * 360, frame rate 15 fps, bitrate 320 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_7_ = 36,
  /** 37: 480 * 360, frame rate 30 fps, bitrate 490 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_360P_8_ = 37,
  /** 38: 640 * 360, frame rate 15 fps, bitrate 800 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_LANDSCAPE_360P_9_ = 38,
  /** 39: 640 * 360, frame rate 24 fps, bitrate 800 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_LANDSCAPE_360P_10_ = 39,
  /** 100: 640 * 360, frame rate 24 fps, bitrate 1000 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_LANDSCAPE_360P_11_ = 100,
  /** 40: 640 * 480, frame rate 15 fps, bitrate 500 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_ = 40,
  /** 42: 480 * 480, frame rate 15 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_3_ = 42,
  /** 43: 640 * 480, frame rate 30 fps, bitrate 750 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_4_ = 43,
  /** 45: 480 * 480, frame rate 30 fps, bitrate 600 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_6_ = 45,
  /** 47: 848 * 480, frame rate 15 fps, bitrate 610 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_8_ = 47,
  /** 48: 848 * 480, frame rate 30 fps, bitrate 930 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_9_ = 48,
  /** 49: 640 * 480, frame rate 10 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_480P_10_ = 49,
  /** 50: 1280 * 720, frame rate 15 fps, bitrate 1130 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_720P_ = 50,
  /** 52: 1280 * 720, frame rate 30 fps, bitrate 1710 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_720P_3_ = 52,
  /** 54: 960 * 720, frame rate 15 fps, bitrate 910 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_720P_5_ = 54,
  /** 55: 960 * 720, frame rate 30 fps, bitrate 1380 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_720P_6_ = 55,
  /** 60: 1920 * 1080, frame rate 15 fps, bitrate 2080 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_1080P_ = 60,
  /** 62: 1920 * 1080, frame rate 30 fps, bitrate 3150 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_1080P_3_ = 62,
  /** 64: 1920 * 1080, frame rate 60 fps, bitrate 4780 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_1080P_5_ = 64,
  /** 66: 2560 * 1440, frame rate 30 fps, bitrate 4850 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_1440P_ = 66,
  /** 67: 2560 * 1440, frame rate 60 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_1440P_2_ = 67,
  /** 70: 3840 * 2160, frame rate 30 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_4K_ = 70,
  /** 72: 3840 * 2160, frame rate 60 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_LANDSCAPE_4K_3_ = 72,
  /** 1000: 120 * 160, frame rate 15 fps, bitrate 65 Kbps. */
  VIDEO_PROFILE_PORTRAIT_120P_ = 1000,
  /** 1002: 120 * 120, frame rate 15 fps, bitrate 50 Kbps. */
  VIDEO_PROFILE_PORTRAIT_120P_3_ = 1002,
  /** 1010: 180 * 320, frame rate 15 fps, bitrate 140 Kbps. */
  VIDEO_PROFILE_PORTRAIT_180P_ = 1010,
  /** 1012: 180 * 180, frame rate 15 fps, bitrate 100 Kbps. */
  VIDEO_PROFILE_PORTRAIT_180P_3_ = 1012,
  /** 1013: 180 * 240, frame rate 15 fps, bitrate 120 Kbps. */
  VIDEO_PROFILE_PORTRAIT_180P_4_ = 1013,
  /** 1020: 240 * 320, frame rate 15 fps, bitrate 200 Kbps. */
  VIDEO_PROFILE_PORTRAIT_240P_ = 1020,
  /** 1022: 240 * 240, frame rate 15 fps, bitrate 140 Kbps. */
  VIDEO_PROFILE_PORTRAIT_240P_3_ = 1022,
  /** 1023: 240 * 424, frame rate 15 fps, bitrate 220 Kbps. */
  VIDEO_PROFILE_PORTRAIT_240P_4_ = 1023,
  /** 1030: 360 * 640, frame rate 15 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_ = 1030,
  /** 1032: 360 * 360, frame rate 15 fps, bitrate 260 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_3_ = 1032,
  /** 1033: 360 * 640, frame rate 30 fps, bitrate 600 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_4_ = 1033,
  /** 1035: 360 * 360, frame rate 30 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_6_ = 1035,
  /** 1036: 360 * 480, frame rate 15 fps, bitrate 320 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_7_ = 1036,
  /** 1037: 360 * 480, frame rate 30 fps, bitrate 490 Kbps. */
  VIDEO_PROFILE_PORTRAIT_360P_8_ = 1037,
  /** 1038: 360 * 640, frame rate 15 fps, bitrate 800 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_PORTRAIT_360P_9_ = 1038,
  /** 1039: 360 * 640, frame rate 24 fps, bitrate 800 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_PORTRAIT_360P_10_ = 1039,
  /** 1100: 360 * 640, frame rate 24 fps, bitrate 1000 Kbps.
   @note `LIVE_BROADCASTING` profile only.
   */
  VIDEO_PROFILE_PORTRAIT_360P_11_ = 1100,
  /** 1040: 480 * 640, frame rate 15 fps, bitrate 500 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_ = 1040,
  /** 1042: 480 * 480, frame rate 15 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_3_ = 1042,
  /** 1043: 480 * 640, frame rate 30 fps, bitrate 750 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_4_ = 1043,
  /** 1045: 480 * 480, frame rate 30 fps, bitrate 600 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_6_ = 1045,
  /** 1047: 480 * 848, frame rate 15 fps, bitrate 610 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_8_ = 1047,
  /** 1048: 480 * 848, frame rate 30 fps, bitrate 930 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_9_ = 1048,
  /** 1049: 480 * 640, frame rate 10 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_PORTRAIT_480P_10_ = 1049,
  /** 1050: 720 * 1280, frame rate 15 fps, bitrate 1130 Kbps. */
  VIDEO_PROFILE_PORTRAIT_720P_ = 1050,
  /** 1052: 720 * 1280, frame rate 30 fps, bitrate 1710 Kbps. */
  VIDEO_PROFILE_PORTRAIT_720P_3_ = 1052,
  /** 1054: 720 * 960, frame rate 15 fps, bitrate 910 Kbps. */
  VIDEO_PROFILE_PORTRAIT_720P_5_ = 1054,
  /** 1055: 720 * 960, frame rate 30 fps, bitrate 1380 Kbps. */
  VIDEO_PROFILE_PORTRAIT_720P_6_ = 1055,
  /** 1060: 1080 * 1920, frame rate 15 fps, bitrate 2080 Kbps. */
  VIDEO_PROFILE_PORTRAIT_1080P_ = 1060,
  /** 1062: 1080 * 1920, frame rate 30 fps, bitrate 3150 Kbps. */
  VIDEO_PROFILE_PORTRAIT_1080P_3_ = 1062,
  /** 1064: 1080 * 1920, frame rate 60 fps, bitrate 4780 Kbps. */
  VIDEO_PROFILE_PORTRAIT_1080P_5_ = 1064,
  /** 1066: 1440 * 2560, frame rate 30 fps, bitrate 4850 Kbps. */
  VIDEO_PROFILE_PORTRAIT_1440P_ = 1066,
  /** 1067: 1440 * 2560, frame rate 60 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_PORTRAIT_1440P_2_ = 1067,
  /** 1070: 2160 * 3840, frame rate 30 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_PORTRAIT_4K_ = 1070,
  /** 1072: 2160 * 3840, frame rate 60 fps, bitrate 6500 Kbps. */
  VIDEO_PROFILE_PORTRAIT_4K_3_ = 1072,
  /** Default 640 * 360, frame rate 15 fps, bitrate 400 Kbps. */
  VIDEO_PROFILE_DEFAULT_ = VIDEO_PROFILE_LANDSCAPE_360P_,
};

enum ORIENTATION_MODE_ {
  /** 0: (Default) Adaptive mode.

   The video encoder adapts to the orientation mode of the video input device.

   - If the width of the captured video from the SDK is greater than the height,
   the encoder sends the video in landscape mode. The encoder also sends the
   rotational information of the video, and the receiver uses the rotational
   information to rotate the received video.
   - When you use a custom video source, the output video from the encoder
   inherits the orientation of the original video. If the original video is in
   portrait mode, the output video from the encoder is also in portrait mode.
   The encoder also sends the rotational information of the video to the
   receiver.
   */
  ORIENTATION_MODE_ADAPTIVE_ = 0,
  /** 1: Landscape mode.

   The video encoder always sends the video in landscape mode. The video encoder
   rotates the original video before sending it and the rotational infomation is
   0. This mode applies to scenarios involving CDN live streaming.
   */
  ORIENTATION_MODE_FIXED_LANDSCAPE_ = 1,
  /** 2: Portrait mode.

   The video encoder always sends the video in portrait mode. The video encoder
   rotates the original video before sending it and the rotational infomation is
   0. This mode applies to scenarios involving CDN live streaming.
   */
  ORIENTATION_MODE_FIXED_PORTRAIT_ = 2,
};

enum FRAME_RATE_ {
  /** 1: 1 fps */
  FRAME_RATE_FPS_1_ = 1,
  /** 7: 7 fps */
  FRAME_RATE_FPS_7_ = 7,
  /** 10: 10 fps */
  FRAME_RATE_FPS_10_ = 10,
  /** 15: 15 fps */
  FRAME_RATE_FPS_15_ = 15,
  /** 24: 24 fps */
  FRAME_RATE_FPS_24_ = 24,
  /** 30: 30 fps */
  FRAME_RATE_FPS_30_ = 30,
  /** 60: 60 fps (Windows and macOS only) */
  FRAME_RATE_FPS_60_ = 60,
};

struct VideoDimensions_ {
  /** Width (pixels) of the video. */
  int width;
  /** Height (pixels) of the video. */
  int height;
  VideoDimensions_() : width(640), height(480) {}
  VideoDimensions_(int w, int h) : width(w), height(h) {}
};

/** Video degradation preferences when the bandwidth is a constraint. */
enum DEGRADATION_PREFERENCE_ {
  /** 0: (Default) Degrade the frame rate in order to maintain the video
     quality. */
  MAINTAIN_QUALITY_ = 0,
  /** 1: Degrade the video quality in order to maintain the frame rate. */
  MAINTAIN_FRAMERATE_ = 1,
  /** 2: (For future use) Maintain a balance between the frame rate and video
     quality. */
  MAINTAIN_BALANCED_ = 2,
};

/** Video mirror modes. */
enum VIDEO_MIRROR_MODE_TYPE_ {
  /** 0: (Default) The SDK enables the mirror mode.
   */
  VIDEO_MIRROR_MODE_AUTO_ = 0,  // determined by SDK
  /** 1: Enable mirror mode. */
  VIDEO_MIRROR_MODE_ENABLED_ = 1,  // enabled mirror
  /** 2: Disable mirror mode. */
  VIDEO_MIRROR_MODE_DISABLED_ = 2,  // disable mirror
};

struct VideoEncoderConfiguration_ {
  /** The video frame dimensions (px) used to specify the video quality and
   * measured by the total number of pixels along a frame's width and height:
   * VideoDimensions. The default value is 640 x 360.
   */
  VideoDimensions_ dimensions;
  /** The frame rate of the video: #FRAME_RATE. The default value is 15.

   Note that we do not recommend setting this to a value greater than 30.
  */
  FRAME_RATE_ frameRate;
  /** The minimum frame rate of the video. The default value is -1.
   */
  int minFrameRate;
  /** The video encoding bitrate (Kbps).

   Choose one of the following options:

   - #STANDARD_BITRATE: (Recommended) The standard bitrate.
      - the `COMMUNICATION` profile: the encoding bitrate equals the base
   bitrate.
      - the `LIVE_BROADCASTING` profile: the encoding bitrate is twice the base
   bitrate.
   - #COMPATIBLE_BITRATE: The compatible bitrate: the bitrate stays the same
   regardless of the profile.

   the `COMMUNICATION` profile prioritizes smoothness, while the
   `LIVE_BROADCASTING` profile prioritizes video quality (requiring a higher
   bitrate). We recommend setting the bitrate mode as #STANDARD_BITRATE to
   address this difference.

   The following table lists the recommended video encoder configurations, where
   the base bitrate applies to the `COMMUNICATION` profile. Set your bitrate
   based on this table. If you set a bitrate beyond the proper range, the SDK
   automatically sets it to within the range.

   @note
   In the following table, **Base Bitrate** applies to the `COMMUNICATION`
   profile, and **Live Bitrate** applies to the `LIVE_BROADCASTING` profile.

   | Resolution             | Frame Rate (fps) | Base Bitrate (Kbps) | Live
   Bitrate (Kbps)                    |
   |------------------------|------------------|----------------------------------------|----------------------------------------|
   | 160 * 120              | 15               | 65 | 130 | | 120 * 120 | 15 |
   50                                     | 100 | | 320 * 180              | 15
   | 140                                    | 280 | | 180 * 180              |
   15               | 100                                    | 200 | | 240 * 180
   | 15               | 120                                    | 240 | | 320 *
   240              | 15               | 200 | 400 | | 240 * 240              |
   15               | 140                                    | 280 | | 424 * 240
   | 15               | 220                                    | 440 | | 640 *
   360              | 15               | 400 | 800 | | 360 * 360              |
   15               | 260                                    | 520 | | 640 * 360
   | 30               | 600                                    | 1200 | | 360 *
   360              | 30               | 400 | 800 | | 480 * 360              |
   15               | 320                                    | 640 | | 480 * 360
   | 30               | 490                                    | 980 | | 640 *
   480              | 15               | 500 | 1000 | | 480 * 480              |
   15               | 400                                    | 800 | | 640 * 480
   | 30               | 750                                    | 1500 | | 480 *
   480              | 30               | 600 | 1200 | | 848 * 480              |
   15               | 610                                    | 1220 | | 848 *
   480              | 30               | 930 | 1860 | | 640 * 480              |
   10               | 400                                    | 800 | | 1280 *
   720             | 15               | 1130                                   |
   2260                                   | | 1280 * 720             | 30 | 1710
   | 3420                                   | | 960 * 720              | 15 |
   910                                    | 1820 | | 960 * 720              | 30
   | 1380                                   | 2760 | | 1920 * 1080            |
   15               | 2080                                   | 4160 | | 1920 *
   1080            | 30               | 3150                                   |
   6300                                   | | 1920 * 1080            | 60 | 4780
   | 6500                                   | | 2560 * 1440            | 30 |
   4850                                   | 6500 | | 2560 * 1440            | 60
   | 6500                                   | 6500 | | 3840 * 2160            |
   30               | 6500                                   | 6500 | | 3840 *
   2160            | 60               | 6500                                   |
   6500                                   |

   */
  int bitrate;
  /** The minimum encoding bitrate (Kbps).

   The SDK automatically adjusts the encoding bitrate to adapt to the network
   conditions. Using a value greater than the default value forces the video
   encoder to output high-quality images but may cause more packet loss and
   hence sacrifice the smoothness of the video transmission. That said, unless
   you have special requirements for image quality, Agora does not recommend
   changing this value.

   @note This parameter applies only to the `LIVE_BROADCASTING` profile.
   */
  int minBitrate;
  /** The video orientation mode of the video: #ORIENTATION_MODE.
   */
  ORIENTATION_MODE_ orientationMode;
  /** The video encoding degradation preference under limited bandwidth:
   * #DEGRADATION_PREFERENCE.
   */
  DEGRADATION_PREFERENCE_ degradationPreference;
  /** Sets the mirror mode of the published local video stream. It only affects
  the video that the remote user sees. See #VIDEO_MIRROR_MODE_TYPE

  @note: The SDK disables the mirror mode by default.
  */
  VIDEO_MIRROR_MODE_TYPE_ mirrorMode;

  VideoEncoderConfiguration_(
      const VideoDimensions_& d, FRAME_RATE_ f, int b, ORIENTATION_MODE_ m,
      VIDEO_MIRROR_MODE_TYPE_ mr = VIDEO_MIRROR_MODE_AUTO_)
      : dimensions(d),
        frameRate(f),
        minFrameRate(-1),
        bitrate(b),
        minBitrate(-1),
        orientationMode(m),
        degradationPreference(MAINTAIN_QUALITY_),
        mirrorMode(mr) {}
  VideoEncoderConfiguration_(
      int width, int height, FRAME_RATE_ f, int b, ORIENTATION_MODE_ m,
      VIDEO_MIRROR_MODE_TYPE_ mr = VIDEO_MIRROR_MODE_AUTO_)
      : dimensions(width, height),
        frameRate(f),
        minFrameRate(-1),
        bitrate(b),
        minBitrate(-1),
        orientationMode(m),
        degradationPreference(MAINTAIN_QUALITY_),
        mirrorMode(mr) {}
  VideoEncoderConfiguration_()
      : dimensions(640, 480),
        frameRate(FRAME_RATE_FPS_15_),
        minFrameRate(-1),
        bitrate(0),
        minBitrate(-1),
        orientationMode(ORIENTATION_MODE_ADAPTIVE_),
        degradationPreference(MAINTAIN_QUALITY_),
        mirrorMode(VIDEO_MIRROR_MODE_AUTO_) {}
};

/**  **DEPRECATED** Definition of the rectangular region. */
typedef struct Rect_ {
  /** Y-axis of the top line.
   */
  int top;
  /** X-axis of the left line.
   */
  int left;
  /** Y-axis of the bottom line.
   */
  int bottom;
  /** X-axis of the right line.
   */
  int right;

  Rect_() : top(0), left(0), bottom(0), right(0) {}
  Rect_(int t, int l, int b, int r) : top(t), left(l), bottom(b), right(r) {}
} Rect_;

#define MAX_CHANNEL_ID_LENGTH_ 65
typedef void* view_t_;

/** Remote video stream types. */
enum REMOTE_VIDEO_STREAM_TYPE_ {
  /** 0: High-stream video. */
  REMOTE_VIDEO_STREAM_HIGH_ = 0,
  /** 1: Low-stream video. */
  REMOTE_VIDEO_STREAM_LOW_ = 1,
};
/** Video display modes. */
enum RENDER_MODE_TYPE_ {
  /**
1: Uniformly scale the video until it fills the visible boundaries (cropped).
One dimension of the video may have clipped contents.
 */
  RENDER_MODE_HIDDEN_ = 1,
  /**
2: Uniformly scale the video until one of its dimension fits the boundary
(zoomed to fit). Areas that are not filled due to disparity in the aspect ratio
are filled with black.
*/
  RENDER_MODE_FIT_ = 2,
  /** **DEPRECATED** 3: This mode is deprecated.
   */
  RENDER_MODE_ADAPTIVE_ = 3,
  /**
  4: The fill mode. In this mode, the SDK stretches or zooms the video to fill
  the display window.
  */
  RENDER_MODE_FILL_ = 4,
};

/** Video display settings of the VideoCanvas class.
 */
struct VideoCanvas_ {
  /** Video display window (view).
   */
  view_t_ view;
  /** The rendering mode of the video view. See #RENDER_MODE_TYPE
   */
  int renderMode;
  /** The unique channel name for the AgoraRTC session in the string format. The
   string length must be less than 64 bytes. Supported character scopes are:
   - All lowercase English letters: a to z.
   - All uppercase English letters: A to Z.
   - All numeric characters: 0 to 9.
   - The space character.
   - Punctuation characters and other symbols, including: "!", "#", "$", "%",
   "&", "(", ")", "+", "-", ":", ";", "<", "=", ".", ">", "?", "@", "[", "]",
   "^", "_", " {", "}", "|", "~", ",".

   @note
   - The default value is the empty string "". Use the default value if the user
   joins the channel using the \ref IRtcEngine::joinChannel "joinChannel" method
   in the IRtcEngine class. The `VideoCanvas` struct defines the video canvas of
   the user in the channel.
   - If the user joins the channel using the \ref IRtcEngine::joinChannel
   "joinChannel" method in the IChannel class, set this parameter as the
   `channelId` of the `IChannel` object. The `VideoCanvas` struct defines the
   video canvas of the user in the channel with the specified channel ID.
   */
  char channelId[MAX_CHANNEL_ID_LENGTH_];
  /** The user ID. */
  uid_t_ uid;
  void* priv;  // private data (underlying video engine denotes it)
  /** The mirror mode of the video view. See VIDEO_MIRROR_MODE_TYPE
   @note
   - For the mirror mode of the local video view: If you use a front camera, the
   SDK enables the mirror mode by default; if you use a rear camera, the SDK
   disables the mirror mode by default.
   - For the mirror mode of the remote video view: The SDK disables the mirror
   mode by default.
  */
  VIDEO_MIRROR_MODE_TYPE_ mirrorMode;

  VideoCanvas_()
      : view(NULL),
        renderMode(RENDER_MODE_HIDDEN_),
        uid(0),
        priv(NULL),
        mirrorMode(VIDEO_MIRROR_MODE_AUTO_) {
    channelId[0] = '\0';
  }
  VideoCanvas_(view_t_ v, int m, uid_t_ u)
      : view(v),
        renderMode(m),
        uid(u),
        priv(NULL),
        mirrorMode(VIDEO_MIRROR_MODE_AUTO_) {
    channelId[0] = '\0';
  }
  VideoCanvas_(view_t_ v, int m, const char* ch, uid_t_ u)
      : view(v),
        renderMode(m),
        uid(u),
        priv(NULL),
        mirrorMode(VIDEO_MIRROR_MODE_AUTO_) {
    strncpy(channelId, ch, MAX_CHANNEL_ID_LENGTH_);
    channelId[MAX_CHANNEL_ID_LENGTH_ - 1] = '\0';
  }
  VideoCanvas_(view_t_ v, int rm, uid_t_ u, VIDEO_MIRROR_MODE_TYPE_ mm)
      : view(v), renderMode(rm), uid(u), priv(NULL), mirrorMode(mm) {
    channelId[0] = '\0';
  }
  VideoCanvas_(view_t_ v, int rm, const char* ch, uid_t_ u,
               VIDEO_MIRROR_MODE_TYPE_ mm)
      : view(v), renderMode(rm), uid(u), priv(NULL), mirrorMode(mm) {
    strncpy(channelId, ch, MAX_CHANNEL_ID_LENGTH_);
    channelId[MAX_CHANNEL_ID_LENGTH_ - 1] = '\0';
  }
};

class IAgoraVideoSourceEngine {
 public:
  virtual void release() = 0;
  virtual bool initialize(const char* appid,
                          IVideoSourceEventHandler* enventHandler) = 0;
  virtual int setChannelProfile(CHANNEL_PROFILE_TYPE_ profile) = 0;
  virtual int setVideoProfile(VIDEO_PROFILE_TYPE_ profile,
                              bool swapWidthAndHeight) = 0;
  virtual int setVideoEncoderConfiguration(
      VideoEncoderConfiguration_ config) = 0;
  virtual int enableWebSdkInteroperability(bool enable) = 0;
  virtual int startScreenCapture(HWND windowId, int capFreq, Rect_ rect,
                                 int bitrate) = 0;
  virtual int stopScreenCapture() = 0;
  virtual int setupLocalVideo(VideoCanvas_ canvas, delegate_render render) = 0;
  virtual int enableVideo(bool enabled) = 0;
  virtual int enableAudio(bool enabled) = 0;
  virtual int enableLocalVideo(bool enabled) = 0;
  virtual int enableLocalAudio(bool enabled) = 0;
  virtual int startPreview(const char* cameraId) = 0;
  virtual int stopPreview() = 0;
  virtual bool enableDualStreamMode(bool enable) = 0;
  virtual int renewToken(const char* token) = 0;
  virtual int joinChannel(const char* token, const char* channelId,
                          const char* info, uid_t_ uid) = 0;
  virtual int leaveChannel() = 0;
  virtual int setCustomRender(bool enable) = 0;
  virtual int setLocalRenderMode(RENDER_MODE_TYPE_ mode) = 0;

  virtual int muteRemoteAudioStream(uid_t_ uid, bool mute) = 0;
  virtual int muteRemoteVideoStream(uid_t_ userId, bool mute) = 0;
  virtual int setRemoteVideoStreamType(
      uid_t_ userId, REMOTE_VIDEO_STREAM_TYPE_ streamType) = 0;
  virtual int muteLocalAudioStream(bool mute) = 0;
  virtual int muteLocalVideoStream(bool mute) = 0;
  virtual int muteAllRemoteAudioStreams(bool mute) = 0;
  virtual int setDefaultMuteAllRemoteAudioStreams(bool mute) = 0;
  virtual int setupRemoteVideo(VideoCanvas_ canvas, delegate_render render) = 0;
};

#define DEVICE_MAX_NUMBER 10
#define DEVICE_MAX_NAME_LENGTH 512
struct Device {
  char id[DEVICE_MAX_NUMBER][DEVICE_MAX_NAME_LENGTH];
  char name[DEVICE_MAX_NUMBER][DEVICE_MAX_NAME_LENGTH];
  int count;
};

/** Properties of the audio volume information.
 An array containing the user ID and volume information for each speaker.
 */
struct AudioVolumeInfo_ {
  /**
   User ID of the speaker. The uid of the local user is 0.
   */
  uid_t_ uid;
  /** The volume of the speaker. The volume ranges between 0 (lowest volume) and
   * 255 (highest volume).
   */
  unsigned int volume;
  /** Voice activity status of the local user.
   * - 0: The local user is not speaking.
   * - 1: The local user is speaking.
   *
   * @note
   * - The `vad` parameter cannot report the voice activity status of the remote
   * users. In the remote users' callback, `vad` = 0.
   * - Ensure that you set `report_vad`(true) in the \ref
   * agora::rtc::IRtcEngine::enableAudioVolumeIndication(int, int, bool)
   * "enableAudioVolumeIndication" method to enable the voice activity detection
   * of the local user.
   */
  unsigned int vad;
  /** The channel ID, which indicates which channel the speaker is in.
   */
  const char* channelId;
};

class IAgoraDeviceEventHandler {
 public:
  virtual void onAudioVolumeIndication(const AudioVolumeInfo_* speakers,
                                       unsigned int speakerNumber,
                                       int totalVolume) = 0;
};

class IAgoraDeviceEngine {
 public:
  virtual void release() = 0;
// size > 1
  virtual bool initialize(const char* appid,size_t size) = 0;
  virtual int getPlaybackDeviceVolume(int* volume) = 0;
  virtual int getRecordingDeviceVolume(int* volume) = 0;
  virtual int enumerateVideoDevices(Device* device) = 0;
  virtual int enumerateRecordingDevices(Device* device) = 0;
  virtual int enumeratePlaybackDevices(Device* device) = 0;
  virtual int setRecordingDevice(
      const char deviceId[DEVICE_MAX_NAME_LENGTH]) = 0;
  virtual int enableAudioVolumeIndication(int interval, int smooth,
                                          bool report_vad) = 0;
  virtual int startRecordingDeviceTest(int indicationInterval) = 0;
  virtual int stopRecordingDeviceTest() = 0;
  virtual int setPlaybackDevice(
      const char deviceId[DEVICE_MAX_NAME_LENGTH]) = 0;
  virtual int startPlaybackDeviceTest(const char* testAudioFilePath) = 0;
  virtual int stopPlaybackDeviceTest() = 0;
  virtual int setDevice(int idx,const char deviceId[DEVICE_MAX_NAME_LENGTH]) = 0;
  virtual int startDeviceTest(int idx,void* hwnd) = 0;
  virtual int stopDeviceTest(int idx) = 0;
  virtual int setRecordingDeviceVolume(int volume) = 0;
  virtual int setPlaybackDeviceVolume(int volume) = 0;
  virtual int registerEventHandler(IAgoraDeviceEventHandler* eventHandler) = 0;
};

}  // namespace rtc
}  // namespace agora

AGORA_API agora::rtc::IAgoraVideoSourceEngine* AGORA_CALL
createAgoraVideoSourceEngine();

AGORA_API agora::rtc::IAgoraDeviceEngine* AGORA_CALL createAgoraDeviceEngine();

#endif