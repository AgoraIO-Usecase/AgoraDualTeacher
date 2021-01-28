#pragma once
#include <vector>
#include <functional>

namespace agora { namespace commons {

class stream_buffer
{
public:
    using data_cb_type = std::function<int(const char *data, size_t length)>;
    stream_buffer(data_cb_type&& data_cb, size_t max_size=10*1024*1024);
    bool receive(const char* data, size_t length);
    size_t get_remaining_data_length() const { return pos_; }
private:
    size_t cache_data(const char* data, size_t length);
    int slice_data(const char* data, size_t length);
private:
    size_t max_size_;
    size_t pos_;
    data_cb_type data_cb_;
    std::vector<char> buffer_;
};
}}
