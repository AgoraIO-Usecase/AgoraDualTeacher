//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "video_local_track_control_packet.h"

#include "engine_adapter/media_engine_manager.h"
#include "engine_adapter/video/video_node_media_control_packet_sender.h"
#include "engine_adapter/video/video_node_network_sink.h"
#include "facilities/tools/api_logger.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[LVCP]";

LocalVideoTrackControlPacketImpl::LocalVideoTrackControlPacketImpl(
    IMediaControlPacketSender* ctrlPacketSender)
    : ctrl_packet_sender_(ctrlPacketSender) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  log(commons::LOG_INFO, "%s: id %d is created ", MODULE_NAME, id_);
}

LocalVideoTrackControlPacketImpl::~LocalVideoTrackControlPacketImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    DetachInfo info;
    info.network = video_network_sink_;
    info.reason = ILocalVideoTrackEx::TRACK_DESTROY;
    doDetach(info);

    ctrl_packet_sender_ = nullptr;
    return 0;
  });
}

bool LocalVideoTrackControlPacketImpl::attach(const AttachInfo& info) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (!media_control_packet_source_) {
    media_control_packet_source_ =
        commons::make_unique<VideoNodeMediaControlPacketSender>(nullptr, ctrl_packet_sender_);
  }

  video_network_sink_ = info.network;
  auto sink = static_cast<VideoNodeNetworkSink*>(video_network_sink_);
  auto sender = static_cast<VideoNodeMediaControlPacketSender*>(media_control_packet_source_.get());
  sender->SetRtpSink(sink);

  return true;
}

bool LocalVideoTrackControlPacketImpl::detach(const DetachInfo& info) { return doDetach(info); }

bool LocalVideoTrackControlPacketImpl::doDetach(const DetachInfo& info) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  if (media_control_packet_source_) {
    auto sender =
        static_cast<VideoNodeMediaControlPacketSender*>(media_control_packet_source_.get());
    sender->SetRtpSink(nullptr);

    video_network_sink_ = nullptr;

    media_control_packet_source_.reset();
  }
  return true;
}

}  // namespace rtc
}  // namespace agora
