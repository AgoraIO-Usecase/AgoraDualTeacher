//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "utils/video_frame_observer.h"

#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"

static const char* const MODULE_NAME = "[RTE.VFO]";

namespace agora {
namespace rte {

LocalVideoFrameObserver::LocalVideoFrameObserver(IAgoraMediaTrack* local_video_track,
                                                 rte::IVideoFrameObserver* video_frame_observer)
    : local_video_track_(local_video_track), video_frame_observer_(video_frame_observer) {
  if (!local_video_track_) {
    LOG_ERR_ASSERT_AND_RET("nullptr local video track in CTOR");
  }

  if (!video_frame_observer_) {
    LOG_ERR_ASSERT_AND_RET("nullptr video fraome observer in CTOR");
  }
}

// TODO(tomiao): currently even removeRenderer() is called, there still might be callback coming
// through video data worker, need to find a solution for it.
// Check WAIT_*_DATA_WORKER_INFLIGHT_TASKS() macros in RTE UT
bool LocalVideoFrameObserver::onCaptureVideoFrame(VideoFrame& video_frame) {
  API_LOGGER_MEMBER("video_frame: %p", &video_frame);

  // don't save stream ID in memory but always get it from track since it might be
  // changed by user from time to time
  char stream_id[kMaxStreamIdSize] = {0};
  if (local_video_track_->GetStreamId(stream_id, kMaxStreamIdSize) != ERR_OK) {
    LOG_ERR_AND_RET_BOOL("failed to get stream ID");
  }

  video_frame_observer_->OnFrame(stream_id, video_frame);
  return true;
}

RemoteVideoFrameObserver::RemoteVideoFrameObserver(const std::string& stream_id,
                                                   rte::IVideoFrameObserver* video_frame_observer)
    : stream_id_(stream_id), video_frame_observer_(video_frame_observer) {
  if (stream_id_.empty()) {
    LOG_ERR_ASSERT_AND_RET("empty stream ID in CTOR");
  }

  if (!video_frame_observer_) {
    LOG_ERR_ASSERT_AND_RET("nullptr video fraome observer in CTOR");
  }
}

bool RemoteVideoFrameObserver::onRenderVideoFrame(rtc::uid_t uid, rtc::conn_id_t conn_id,
                                                  VideoFrame& video_frame) {
  API_LOGGER_MEMBER("uid: %u, conn_id: %u, video_frame: %p", uid, conn_id, &video_frame);

  // have got stream ID in CTOR, so no need to convert it from uid here
  video_frame_observer_->OnFrame(stream_id_.c_str(), video_frame);
  return true;
}

}  // namespace rte
}  // namespace agora
