
//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019-2020 Agora IO. All rights reserved.
//

#include "remote_track_manager.h"

#include "api2/internal/local_user_i.h"
#include "api2/internal/video_track_i.h"
#include "base/base_type.h"
#include "base/user_id_manager.h"
#include "call_engine/call_context.h"
#include "channel_manager.h"
#include "channel_proxy.h"
#include "engine_adapter/video/video_codec_map.h"
#include "facilities/tools/api_logger.h"
#include "internal/rtc_engine_i.h"
#include "rtc_engine_impl.h"

const char MODULE_NAME[] = "[CHP]";

namespace agora {
namespace rtc {

RemoteTrackManager::RemoteTrackManager() = default;

// RemoteTrackManager is called by multiple thread, need mutex for time being.
RemoteTrackManager::~RemoteTrackManager() {
  remote_tracks_view_.clear();
  remote_tracks_renderer_.clear();
  remote_video_track_.clear();
}

void RemoteTrackManager::addRemoteVideoTrack(uid_t uid,
                                             agora_refptr<rtc::IRemoteVideoTrack> videoTrack) {
  std::lock_guard<std::mutex> l(map_mutex_);
  remote_video_track_[uid].emplace(videoTrack);
}

void RemoteTrackManager::removeRemoteVideoTrack(uid_t uid,
                                                agora_refptr<rtc::IRemoteVideoTrack> videoTrack) {
  std::lock_guard<std::mutex> l(map_mutex_);
  auto it = remote_video_track_.find(uid);
  if (it == remote_video_track_.end()) {
    log(LOG_INFO, "%s remove remote video track : No find uid %d", MODULE_NAME, uid);
    return;
  }

  auto trackIt = remote_video_track_[uid].find(videoTrack);
  if (trackIt == remote_video_track_[uid].end()) {
    log(LOG_INFO, "%s remove remote video track : No find video track %d", MODULE_NAME, uid);
    return;
  }

  remote_video_track_[uid].erase(trackIt);
  if (!remote_video_track_[uid].size()) {
    remote_video_track_.erase(uid);
  }
}

void RemoteTrackManager::setRemoteTrackRenderer(uid_t uid, uint32_t trackId,
                                                agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                                                agora_refptr<rtc::IVideoRenderer> remoteRenderer) {
  RendererMap::mapped_type previousRender;
  {
    std::lock_guard<std::mutex> l(map_mutex_);
    RendererMap::mapped_type previousRender = remote_tracks_renderer_[{uid, trackId}];
    remote_tracks_renderer_[{uid, trackId}] = remoteRenderer;

    auto it = remote_tracks_view_.find({uid, trackId});
    if (it != remote_tracks_view_.end()) {
      if (previousRender) {
        // https://jira.agoralab.co/browse/MS-10333
        // unbind view explicitly before switching to the new renderer
        (static_cast<IVideoRendererEx*>(previousRender.get()))->setViewEx(utils::kInvalidHandle);
      }
      // View has been set before video track susbscribed
      (static_cast<IVideoRendererEx*>(remoteRenderer.get()))->setViewEx(it->second.first);
      remoteRenderer->setRenderMode(it->second.second);
    }
  }

  // the destruction of previousRender must happen in major worker
  // and previousRender should never be referenced afterward
  if (previousRender) {
    videoTrack->removeRenderer(std::move(previousRender));
  }
  videoTrack->addRenderer(remoteRenderer);
}

void RemoteTrackManager::removeRemoteViews(uid_t uid) {
  std::lock_guard<std::mutex> l(map_mutex_);
  for (auto& elem : remote_tracks_view_) {
    if (elem.first.first == uid) {
      remote_tracks_renderer_.erase(elem.first);
      log(LOG_INFO, "%s remove remote video track : No find video track %d", MODULE_NAME, uid);
    }
  }
}

void RemoteTrackManager::getRemoteVideoTrackStats(uid_t uid, RemoteVideoTrackStats& stats) {
  std::set<agora_refptr<rtc::IRemoteVideoTrack>> track_snapshot;
  {
    std::lock_guard<std::mutex> l(map_mutex_);
    copyTracks(uid, false, track_snapshot);
  }
  for (auto& track : track_snapshot) {
    bool success = track->getStatistics(stats);
    if (!success || (stats.uid == 0) || (stats.receivedBitrate == 0)) {
      continue;
    }
  }
}

void RemoteTrackManager::clearRemoteViews() {
  std::lock_guard<std::mutex> l(map_mutex_);
  remote_tracks_view_.clear();
  remote_tracks_renderer_.clear();
}

void RemoteTrackManager::removeRemoteTrackView(TrackPair track) {
  std::lock_guard<std::mutex> l(map_mutex_);
  remote_tracks_renderer_.erase(track);
  remote_tracks_view_.erase(track);
}

bool RemoteTrackManager::setRemoteView(TrackPair track, view_t view) {
  std::lock_guard<std::mutex> l(map_mutex_);
  utils::object_handle handle = ViewToHandle(view);
  remote_tracks_view_[track].first = handle;
  auto it = remote_tracks_renderer_.find(track);
  if (it != remote_tracks_renderer_.end()) {
    (static_cast<IVideoRendererEx*>(it->second.get()))->setViewEx(handle);
  }
  return true;
}

int RemoteTrackManager::setRemoteRenderMode(TrackPair track,
                                            media::base::RENDER_MODE_TYPE renderMode) {
  std::lock_guard<std::mutex> l(map_mutex_);
  remote_tracks_view_[track].second = renderMode;
  auto it = remote_tracks_renderer_.find(track);
  if (it != remote_tracks_renderer_.end()) {
    return it->second->setRenderMode(renderMode);
  }
  return ERR_OK;
}

int RemoteTrackManager::registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) {
  std::set<agora_refptr<rtc::IRemoteVideoTrack>> track_snapshot;
  {
    std::lock_guard<std::mutex> l(map_mutex_);
    copyTracks(0, true, track_snapshot);
  }
  for (auto& track : track_snapshot) {
    if (track) track->registerVideoEncodedImageReceiver(receiver);
  }
  return ERR_OK;
}

int RemoteTrackManager::unregisterVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver) {
  std::set<agora_refptr<rtc::IRemoteVideoTrack>> track_snapshot;
  {
    std::lock_guard<std::mutex> l(map_mutex_);
    copyTracks(0, true, track_snapshot);
  }
  for (auto& track : track_snapshot) {
    if (track) track->unregisterVideoEncodedImageReceiver(receiver);
  }
  return ERR_OK;
}

void RemoteTrackManager::copyTracks(uid_t uid, bool copyAll,
                                    std::set<agora_refptr<rtc::IRemoteVideoTrack>>& tracks) {
  if (!copyAll) {
    tracks = remote_video_track_[uid];
    return;
  }
  for (auto& pair : remote_video_track_) {
    tracks.insert(pair.second.begin(), pair.second.end());
  }
}

}  // namespace rtc
}  // namespace agora
