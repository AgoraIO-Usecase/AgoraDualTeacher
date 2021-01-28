#pragma once
#include "edu_video_frame_impl.h"
#include <memory>
namespace agora {
namespace edu {
EduVideoFrame::EduVideoFrame() {}
EduVideoFrame::EduVideoFrame(image_frame_info header, unsigned short width,
                             unsigned short height, unsigned char* stride_y,
                             int stride_y_size, unsigned char* stride_u,
                             int stride_u_size, unsigned char* stride_v,
                             int sride_v_size, unsigned int timestamp)
    : width_(width),
      height_(height),
      stride_y_size_(stride_y_size),
      stride_u_size_(stride_u_size),
      stride_v_size_(sride_v_size),
      timestamp_(timestamp)

{
  header_ = header;
  int size = stride_y_size_ * 1.5 * height_;
  auto buf = new unsigned char[size];
  std::memcpy(buf, stride_y, size);
  buffer_.reset(buf);
}

EduVideoFrame::EduVideoFrame(const EduVideoFrame& lrs)
    : width_(lrs.width_),
      height_(lrs.height_),
      stride_y_size_(lrs.stride_y_size_),
      stride_u_size_(lrs.stride_u_size_),
      stride_v_size_(lrs.stride_v_size_),
      timestamp_(lrs.timestamp_) {
  buffer_ = lrs.buffer_;
  header_ = lrs.header_;
}
image_frame_info EduVideoFrame::getImageInfo() { return header_; }
IEduVideoFrame::VIDEO_TYPE_ EduVideoFrame::GetVideoType() const {
  return VIDEO_TYPE_YV12;
}
void EduVideoFrame::release() { buffer_.reset(); }
int EduVideoFrame::getRotation() const { return header_.rotation; }
bool EduVideoFrame::getMirror() const { return header_.mirrored; }
/*!
 * @brief
 * @author LuCheng
 * @date   2020/11/5
 * @param  PLANE_TYPE type
 * @return     const unsigned char*
 */
const unsigned char* EduVideoFrame::buffer(PLANE_TYPE_ type) const {
  switch (type) {
    case Y_PLANE:
      return buffer_.get();
    case U_PLANE:
      return buffer_.get() + allocated_size(Y_PLANE);
    case V_PLANE:
      return buffer_.get() + allocated_size(Y_PLANE) + allocated_size(U_PLANE);
  }
}

// Copy frame: If required size is bigger than allocated one, new buffers of
// adequate size will be allocated.
// Return value: 0 on success ,-1 on error.
int EduVideoFrame::copyFrame(IEduVideoFrame** dest_frame) const {
  if (!dest_frame) return -1;
  if (!*dest_frame) {
    *dest_frame = new EduVideoFrame(
        header_, width_, height_,
        (unsigned char*)buffer(IEduVideoFrame::Y_PLANE), stride_y_size_,
        (unsigned char*)buffer(IEduVideoFrame::U_PLANE), stride_u_size_,
        (unsigned char*)buffer(IEduVideoFrame::V_PLANE), stride_v_size_,
        timestamp_);
    return 0;
  }

  EduVideoFrame* video_frame = dynamic_cast<EduVideoFrame*>(*dest_frame);
  if (video_frame->allocated_size(EduVideoFrame::Y_PLANE) >
      this->allocated_size(EduVideoFrame::Y_PLANE)) {
    video_frame->release();
    video_frame = new EduVideoFrame(*this);
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
int EduVideoFrame::convertFrame(VIDEO_TYPE_ dst_video_type, int dst_sample_size,
                                unsigned char* dst_frame) const {
  return -1;
}

// Get allocated size per plane.
int EduVideoFrame::allocated_size(PLANE_TYPE_ type) const {
  switch (type) {
    case EduVideoFrame::Y_PLANE:
      return stride_y_size_ * height_;
    case EduVideoFrame::U_PLANE:
      return stride_y_size_ * height_ / 4;
    case EduVideoFrame::V_PLANE:
      return stride_y_size_ * height_ / 4;
  }
  return height_ * width_ * 1.5;
}

// Get allocated stride per plane.
int EduVideoFrame::stride(PLANE_TYPE_ type) const {
  switch (type) {
    case EduVideoFrame::Y_PLANE:
      return stride_y_size_;
    case EduVideoFrame::U_PLANE:
      return stride_u_size_;
    case EduVideoFrame::V_PLANE:
      return stride_v_size_;
  }
  return stride_y_size_;
}

// Get frame width.
int EduVideoFrame::width() const { return width_; }

// Get frame height.
int EduVideoFrame::height() const { return height_; }
// Get frame timestamp (90kHz).
unsigned int EduVideoFrame::timestamp() const { return 0; }

// Get render time in milliseconds.
int64_t EduVideoFrame::render_time_ms() const { return timestamp_; }

// Return true if underlying plane buffers are of zero size, false if not.
bool EduVideoFrame::IsZeroSize() const { return buffer_.get(); }

}  // namespace edu
}  // namespace agora
