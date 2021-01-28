//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-02.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "video_remote_track_ctrl_packet.h"

#include "engine_adapter/video/video_node_control_packet_source.h"
#include "engine_adapter/video/video_node_internal.h"
#include "facilities/tools/api_logger.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

using agora::commons::log;
const char MODULE_NAME[] = "[RVTP]";

RemoteVideoTrackCtrlPacketImpl::RemoteVideoTrackCtrlPacketImpl() : video_network_source_(nullptr) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    control_worker_ = utils::minor_worker("RemotePipeLineWorkerCtrlPacket");
    control_packet_source_ = commons::make_unique<VideoNodeControlPacketSource>(control_worker_);
    return 0;
  });
}

RemoteVideoTrackCtrlPacketImpl::~RemoteVideoTrackCtrlPacketImpl() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] { return 0; });
}

uint32_t RemoteVideoTrackCtrlPacketImpl::getRemoteSsrc() { return 0; }

void RemoteVideoTrackCtrlPacketImpl::OnNodeActive(VideoNodeBase* node) {}

void RemoteVideoTrackCtrlPacketImpl::OnNodeInactive(VideoNodeBase* node) {}

void RemoteVideoTrackCtrlPacketImpl::OnNodeWillStart(VideoNodeBase* node) {}

void RemoteVideoTrackCtrlPacketImpl::OnNodeWillDestroy(VideoNodeBase* node) {
  utils::major_worker()->sync_call(LOCATION_HERE, [this, node] {
    // network source or rtcp sink will call this
    DetachInfo info;
    info.reason = NETWORK_DESTROY;
    info.source = video_network_source_;

    detach(info, REMOTE_VIDEO_STATE_REASON_INTERNAL);
    return 0;
  });
}

bool RemoteVideoTrackCtrlPacketImpl::attach(const AttachInfo& info,
                                            REMOTE_VIDEO_STATE_REASON reason) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, info] {
    video_network_source_ = info.source;
    if (video_network_source_) {
      video_network_source_->AddStateListener(this);
      if (control_packet_source_->getMediaControlPacketReceiverNumber() > 0) {
        auto network_source = static_cast<VideoNodeNetworkSource*>(video_network_source_);
        network_source->registerRtcpPacketSink(control_packet_source_.get());
      }
    }
    return 0;
  }) == 0;
}

bool RemoteVideoTrackCtrlPacketImpl::detach(const DetachInfo& info,
                                            REMOTE_VIDEO_STATE_REASON reason) {
  return doDetach(info, reason);
}

bool RemoteVideoTrackCtrlPacketImpl::doDetach(const DetachInfo& info,
                                              REMOTE_VIDEO_STATE_REASON reason) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, info] {
    if (video_network_source_) {
      auto network_source = static_cast<VideoNodeNetworkSource*>(video_network_source_);
      network_source->unregisterRtcpPacketSink(control_packet_source_.get());

      video_network_source_->RemoveStateListener(this);
      video_network_source_ = nullptr;
    }
    return 0;
  }) == 0;
}

int RemoteVideoTrackCtrlPacketImpl::registerMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, ctrlPacketReceiver]() {
    if (ctrlPacketReceiver) {
      control_packet_source_->registerMediaControlPacketReceiver(ctrlPacketReceiver);
      if (control_packet_source_->getMediaControlPacketReceiverNumber() == 1 &&
          video_network_source_) {
        auto network_source = static_cast<VideoNodeNetworkSource*>(video_network_source_);
        network_source->registerRtcpPacketSink(control_packet_source_.get());
      }
    }
    return ERR_OK;
  });
}

int RemoteVideoTrackCtrlPacketImpl::unregisterMediaControlPacketReceiver(
    IMediaControlPacketReceiver* ctrlPacketReceiver) {
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, ctrlPacketReceiver]() {
    if (ctrlPacketReceiver) {
      control_packet_source_->unregisterMediaControlPacketReceiver(ctrlPacketReceiver);
      if (control_packet_source_->getMediaControlPacketReceiverNumber() == 0 &&
          video_network_source_) {
        auto network_source = static_cast<VideoNodeNetworkSource*>(video_network_source_);
        network_source->unregisterRtcpPacketSink(control_packet_source_.get());
      }
    }
    return ERR_OK;
  });
}

}  // namespace rtc
}  // namespace agora
