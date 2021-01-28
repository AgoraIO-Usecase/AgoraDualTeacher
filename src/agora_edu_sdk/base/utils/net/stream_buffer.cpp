#include "stream_buffer.h"
#include <cassert>
#include <cstring>

namespace agora {
namespace commons {
stream_buffer::stream_buffer(data_cb_type&& data_cb, size_t max_size)
    : max_size_(max_size), pos_(0), data_cb_(std::move(data_cb)) {}

size_t stream_buffer::cache_data(const char* data, size_t length) {
  size_t num = pos_ + length;
  if (pos_ == max_size_)
    return 0;
  else if (num > max_size_) {
    num = max_size_;
    length = num - pos_;
    buffer_.resize(num);
  } else if (num > buffer_.size()) {
    buffer_.resize(num);
  }
  std::copy(data, data + length, buffer_.begin() + pos_);
  pos_ = num;
  return length;
}

bool stream_buffer::receive(const char* data, size_t length) {
  if (!data || !length) return false;
  if (!data_cb_) return false;
  int r = 0;
  if (pos_) {
    int pos = 0;
    (pos);
    size_t cached = cache_data(data, length);
    if (!cached) return false;  // cannot cache enough data
    r = slice_data(&buffer_[0], pos_);
    assert(r <= (int)pos_);
    if (r < 0)
      return false;
    else if (r == 0)
      return true;
    else if (pos_ > static_cast<size_t>(r)) {
      // move remaining bytes to the begining
      pos_ -= r;
      if (pos_ > 0) memmove(&buffer_[0], &buffer_[r], pos_);
      if (cached < length)  // handle remainings
        return receive(data + cached, length - cached);
    } else  // r > pos_, should not be here
      pos_ = 0;
  } else {
    r = slice_data(data, length);
    assert(r <= (int)length);
    if (r < 0) return false;
    if (length - r > 0) cache_data(data + r, length - r);
  }
  return true;
}

int stream_buffer::slice_data(const char* data, size_t length) {
  int handled = 0;
  while (length > 0) {
    int r = data_cb_(data + handled, length);
    assert(r <= (int)length);
    if (r < 0)
      return r;  // error
    else if (r == 0)
      return handled;  // need cache
    else if (r < (int)length) {
      // try next piece
      handled += r;
      length -= r;
    } else {
      // nothing left
      handled += length;
      length = 0;
    }
  }
  return handled;
}
}  // namespace commons
}  // namespace agora
