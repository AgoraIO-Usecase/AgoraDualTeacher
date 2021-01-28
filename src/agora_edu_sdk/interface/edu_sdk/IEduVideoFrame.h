#pragma once
#include <cstdint>
namespace agora {
namespace edu {
class IEduVideoFrame {
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
  virtual VIDEO_TYPE_ GetVideoType() const = 0;
  virtual void release() = 0;
  virtual const unsigned char* buffer(PLANE_TYPE_ type) const = 0;
  // Copy frame: If required size is bigger than allocated one, new buffers of
  // adequate size will be allocated.
  // Return value: 0 on success ,-1 on error.
  virtual int copyFrame(IEduVideoFrame** dest_frame) const = 0;
  // Convert frame
  // Input:
  //   - src_frame        : Reference to a source frame.
  //   - dst_video_type   : Type of output video.
  //   - dst_sample_size  : Required only for the parsing of MJPG.
  //   - dst_frame        : Pointer to a destination frame.
  // Return value: 0 if OK, < 0 otherwise.
  // It is assumed that source and destination have equal height.
  virtual int convertFrame(VIDEO_TYPE_ dst_video_type, int dst_sample_size,
                           unsigned char* dst_frame) const = 0;

  // Get allocated size per plane.
  virtual int allocated_size(PLANE_TYPE_ type) const = 0;

  // Get allocated stride per plane.
  virtual int stride(PLANE_TYPE_ type) const = 0;

  // Get frame width.
  virtual int width() const = 0;

  // Get frame height.
  virtual int height() const = 0;
  // Get frame timestamp (90kHz).
  virtual unsigned int timestamp() const = 0;

  // Get render time in milliseconds.
  virtual int64_t render_time_ms() const = 0;

  // Return true if underlying plane buffers are of zero size, false if not.
  virtual bool IsZeroSize() const = 0;

  virtual int getRotation() const = 0;
  virtual bool getMirror() const = 0;
};

}  // namespace edu
}  // namespace agora
