//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "video_local_track_packet.h"

#include "engine_adapter/video/video_node_media_packet_sender.h"
#include "engine_adapter/video/video_node_network_sink.h"
#include "facilities/tools/api_logger.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

static const char MODULE_NAME[] = "[LVP]";

LocalVideoTrackPacketImpl::LocalVideoTrackPacketImpl(agora_refptr<IMediaPacketSender> videoSource)
    : video_source_(videoSource) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, videoSource] {
    media_packet_source_ = std::make_shared<VideoNodeMediaPacketSender>(nullptr, videoSource);
    if (!media_packet_source_) {
      log(agora::commons::LOG_FATAL, "%s: failed: create media packet source failed", MODULE_NAME);
      return -ERR_FAILED;
    }
    return 0;
  });
}

LocalVideoTrackPacketImpl::~LocalVideoTrackPacketImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    DetachInfo info;
    info.network = video_network_sink_;
    info.reason = ILocalVideoTrackEx::TRACK_DESTROY;
    detach(info);
    media_packet_source_.reset();
    video_source_.reset();
    return 0;
  });
}

bool LocalVideoTrackPacketImpl::attach(const AttachInfo& info) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &info] {
    video_network_sink_ = info.network;

    if (video_source_ && video_network_sink_) {
      VideoNodeMediaPacketSender* media_packet_sender =
          static_cast<VideoNodeMediaPacketSender*>(media_packet_source_.get());
      VideoNodeNetworkSink* sink = static_cast<VideoNodeNetworkSink*>(video_network_sink_);
      media_packet_sender->SetRtpSink(sink);
    }
    return 0;
  }) == 0;
}

bool LocalVideoTrackPacketImpl::detach(const DetachInfo& info) { return doDetach(info); }

bool LocalVideoTrackPacketImpl::doDetach(const DetachInfo& info) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, &info] {
    VideoNodeMediaPacketSender* media_packet_sender =
        static_cast<VideoNodeMediaPacketSender*>(media_packet_source_.get());
    media_packet_sender->SetRtpSink(nullptr);

    if (video_network_sink_) {
      video_network_sink_ = nullptr;
    }
    return 0;
  }) == 0;
}

}  // namespace rtc
}  // namespace agora
