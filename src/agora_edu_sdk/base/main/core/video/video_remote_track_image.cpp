//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_remote_track_image.h"
#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_decoder.h"
#include "facilities/tools/api_logger.h"
#include "wrappers/video_custom_decoder_wrapper.h"
#include "wrappers/video_decoder_wrapper.h"

namespace agora {
namespace rtc {

RemoteVideoTrackImageImpl::RemoteVideoTrackImageImpl(
    const RemoteVideoTrackImpl::RemoteVideoTrackConfig& config)
    : RemoteVideoTrackImpl(config) {}

RemoteVideoTrackImageImpl::~RemoteVideoTrackImageImpl() {}

std::shared_ptr<rtc::VideoNodeDecoder> RemoteVideoTrackImageImpl::createVideoRxProcessor(
    utils::worker_type worker, uint8_t payload) {
  if (!rtc::RtcGlobals::Instance().EngineManager()) return nullptr;

  auto decoder = rtc::RtcGlobals::Instance().EngineManager()->VideoEngine().CreateVideoRxProcessor(
      worker, true, payload);
  return decoder;
}

int RemoteVideoTrackImageImpl::registerVideoEncodedImageReceiver(
    IVideoEncodedImageReceiver* videoReceiver) {
  API_LOGGER_MEMBER("videoReceiver:%p", videoReceiver);

  if (!videoReceiver) {
    return -1;
  }
  auto rx_processor = static_cast<rtc::VideoNodeRxProcessor*>(decoder_.get());
  auto customDecoderWrapper =
      static_cast<rtc::VideoCustomDecoderWrapper*>(rx_processor->RawDecoder()->RealDecoder());
  customDecoderWrapper->registerVideoEncodedImageReceiver(videoReceiver);
  return 0;
}

int RemoteVideoTrackImageImpl::unregisterVideoEncodedImageReceiver(
    IVideoEncodedImageReceiver* videoReceiver) {
  API_LOGGER_MEMBER("videoReceiver:%p", videoReceiver);

  if (!videoReceiver) {
    return -1;
  }
  rtc::VideoNodeRxProcessor* rx = static_cast<rtc::VideoNodeRxProcessor*>(decoder_.get());
  if (!rx) {
    log(commons::LOG_ERROR, "decoder not exist");
    return -1;
  }
  auto customDecoderWrapper =
      static_cast<rtc::VideoCustomDecoderWrapper*>(rx->RawDecoder()->RealDecoder());
  customDecoderWrapper->unregisterVideoEncodedImageReceiver(videoReceiver);
  return 0;
}

}  // namespace rtc
}  // namespace agora
