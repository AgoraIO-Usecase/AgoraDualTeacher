#pragma once
#include <memory>
#include "interface/edu_sdk/IEduVideoFrame.h"
namespace agora {
namespace edu {

struct image_frame_info {
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

class EduVideoFrame : public IEduVideoFrame {
 public:
  EduVideoFrame();
  EduVideoFrame(image_frame_info header, unsigned short width,
                unsigned short height, unsigned char* stride_y,
                int stride_y_size, unsigned char* stride_u, int stride_u_size,
                unsigned char* stride_v, int sride_v_size,
                unsigned int timestamp);
  EduVideoFrame(const EduVideoFrame& lrs);

  image_frame_info getImageInfo();

  virtual VIDEO_TYPE_ GetVideoType() const override;
  virtual void release() override;

  virtual int getRotation() const override;
  virtual bool getMirror() const override;
  virtual const unsigned char* buffer(PLANE_TYPE_ type) const override;

  // Copy frame: If required size is bigger than allocated one, new buffers of
  // adequate size will be allocated.
  // Return value: 0 on success ,-1 on error.
  virtual int copyFrame(IEduVideoFrame** dest_frame) const override;

  // Convert frame
  // Input:
  //   - src_frame        : Reference to a source frame.
  //   - dst_video_type   : Type of output video.
  //   - dst_sample_size  : Required only for the parsing of MJPG.
  //   - dst_frame        : Pointer to a destination frame.
  // Return value: 0 if OK, < 0 otherwise.
  // It is assumed that source and destination have equal height.
  virtual int convertFrame(VIDEO_TYPE_ dst_video_type, int dst_sample_size,
                           unsigned char* dst_frame) const override;

  // Get allocated size per plane.
  virtual int allocated_size(PLANE_TYPE_ type) const override;
  // Get allocated stride per plane.
  virtual int stride(PLANE_TYPE_ type) const override;

  // Get frame width.
  virtual int width() const override;

  // Get frame height.
  virtual int height() const override;
  // Get frame timestamp (90kHz).
  virtual unsigned int timestamp() const override;

  // Get render time in milliseconds.
  virtual int64_t render_time_ms() const override;

  // Return true if underlying plane buffers are of zero size, false if not.
  virtual bool IsZeroSize() const override;

 private:
  image_frame_info header_;
  int width_;
  int height_;
  std::shared_ptr<unsigned char> buffer_;
  int stride_y_size_;
  int stride_u_size_;
  int stride_v_size_;
  int timestamp_;
};

}  // namespace edu
}  // namespace agora
