//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "video_frame_metadata_observer.h"

#include "utils/log/log.h"

namespace agora {
namespace rtc {

namespace {
constexpr char MODULE_NAME[] = "[VMO]";
constexpr int MINIMAL_SENDING_INTERNAL = 1000 / 15;
}  // namespace

VideoMetadataObserverImpl::VideoMetadataObserverImpl(
    utils::RtcSteadySyncCallback<agora::rtc::IMetadataObserver>::SharedType send_callback,
    utils::RtcAsyncCallback<agora::rtc::IMetadataObserver>::Type recv_callback,
    VideoTrackInfo trackInfo)
    : send_callback_(send_callback), recv_callback_(recv_callback), track_info_(trackInfo) {
  last_send_timestamp_ms_ = ::rtc::TimeMillis() - MINIMAL_SENDING_INTERNAL;
}

VideoMetadataObserverImpl::~VideoMetadataObserverImpl() {}

bool VideoMetadataObserverImpl::adaptVideoFrame(const webrtc::VideoFrame& inputFrame,
                                                webrtc::VideoFrame& outputFrame) {
  outputFrame = inputFrame;

  if (!enable_) {
    return true;
  }
  outputFrame.set_metadata(std::vector<uint8_t>());

  if (IMetadataObserver::INVALID_METADATA_SIZE_IN_BYTE == max_metadata_size_in_byte_) {
    send_callback_->Call([this](auto event_handler) {
      max_metadata_size_in_byte_ = event_handler->getMaxMetadataSize();
    });
    log(agora::commons::LOG_STREAM, "%s: getMaxMetadataSize result:%d", MODULE_NAME,
        max_metadata_size_in_byte_);
  }

  if (max_metadata_size_in_byte_ < 0 ||
      max_metadata_size_in_byte_ > IMetadataObserver::MAX_METADATA_SIZE_IN_BYTE) {
    log(agora::commons::LOG_STREAM, "%s: invalid max_metadata_size_, err:%d", MODULE_NAME,
        max_metadata_size_in_byte_);
    return true;
  }

  // local track
  if (0 == track_info_.ownerUid) {
    onReadyToSendMetadata(outputFrame);
  } else {
    onMetadataReceived(inputFrame);
  }

  // need to pass down the frame anyway
  return true;
}

void VideoMetadataObserverImpl::onReadyToSendMetadata(webrtc::VideoFrame& outputFrame) {
  auto now_ms = ::rtc::TimeMillis();
  if (now_ms - last_send_timestamp_ms_ < MINIMAL_SENDING_INTERNAL) {
    log(agora::commons::LOG_STREAM,
        "%s: drop this meta info due to last sending occurs within %d ms", MODULE_NAME,
        MINIMAL_SENDING_INTERNAL);
    return;
  }

  bool need_send = false;
  IMetadataObserver::Metadata meta_data = {0};
  meta_data.timeStampMs = now_ms;
  std::vector<uint8_t> meta_buffer(max_metadata_size_in_byte_);
  meta_data.buffer = (unsigned char*)(meta_buffer.data());
  send_callback_->Call(
      [&](auto event_handler) { need_send = event_handler->onReadyToSendMetadata(meta_data); });
  if (!need_send || 0 == meta_data.size) {
    return;
  }
  if (meta_data.size > max_metadata_size_in_byte_) {
    log(agora::commons::LOG_STREAM, "%s: truncated", MODULE_NAME);
    meta_data.size = max_metadata_size_in_byte_;
  }

  // need to append SEI info
  std::vector<uint8_t> cut_buffer(meta_buffer.data(), meta_buffer.data() + meta_data.size);
  outputFrame.set_metadata(std::move(cut_buffer));
  last_send_timestamp_ms_ = now_ms;
}

void VideoMetadataObserverImpl::onMetadataReceived(const webrtc::VideoFrame& outputFrame) {
  // report SEI info
  auto meta_buffer = outputFrame.get_metadata();
  if (meta_buffer.size() > max_metadata_size_in_byte_) {
    log(agora::commons::LOG_STREAM,
        "%s: received a meta buffer with size %d, larger than max size %d", MODULE_NAME,
        meta_buffer.size(), max_metadata_size_in_byte_);
    return;
  }
  if (0 == meta_buffer.size()) {
    log(agora::commons::LOG_STREAM, "%s: received a meta buffer with zero size", MODULE_NAME);
    return;
  }

  auto uid = track_info_.ownerUid;
  auto ntp_time_ms = outputFrame.ntp_time_ms();
  recv_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    IMetadataObserver::Metadata meta_data = {0};
    meta_data.uid = uid;
    meta_data.timeStampMs = ntp_time_ms;
    meta_data.size = meta_buffer.size();
    meta_data.buffer = const_cast<unsigned char*>(meta_buffer.data());
    event_handler->onMetadataReceived(meta_data);
  });
}

}  // namespace rtc
}  // namespace agora
