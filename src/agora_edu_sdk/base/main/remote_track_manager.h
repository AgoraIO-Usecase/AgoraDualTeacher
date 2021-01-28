//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019-2020 Agora IO. All rights reserved.
//
#pragma once

#include <map>
#include <mutex>

#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/rtc_connection_i.h"
#include "facilities/miscellaneous/view_manager.h"
#include "facilities/tools/rtc_callback.h"
//#include "internal/rtc_engine_i.h"
#include "utils/packer/packet.h"

namespace agora {
namespace rtc {

class RemoteTrackManager {
 protected:
  struct TrackComparator {
    inline bool operator()(const agora_refptr<rtc::IRemoteVideoTrack>& p1,
                           const agora_refptr<rtc::IRemoteVideoTrack>& p2) const {
      return p1.get() > p2.get();
    }
  };

  using TrackPair = std::pair<uid_t, track_id_t>;
  using ViewPair = std::pair<utils::object_handle, media::base::RENDER_MODE_TYPE>;
  using ViewMap = std::map<TrackPair, ViewPair>;
  using RendererMap = std::map<TrackPair, agora_refptr<rtc::IVideoRenderer>>;
  using VideoTrackMap = std::map<uid_t, std::set<agora_refptr<rtc::IRemoteVideoTrack>>>;

 public:
  RemoteTrackManager();
  ~RemoteTrackManager();
  void setRemoteTrackRenderer(uid_t uid, track_id_t trackId,
                              agora_refptr<rtc::IRemoteVideoTrack> videoTrack,
                              agora_refptr<rtc::IVideoRenderer> remoteRenderer);
  bool setRemoteView(TrackPair track, view_t view);
  int setRemoteRenderMode(TrackPair track, media::base::RENDER_MODE_TYPE renderMode);
  void removeRemoteViews(uid_t uid);
  void clearRemoteViews();
  void removeRemoteTrackView(TrackPair track);
  void addRemoteVideoTrack(uid_t uid, agora_refptr<rtc::IRemoteVideoTrack> videoTrack);
  void removeRemoteVideoTrack(uid_t uid, agora_refptr<rtc::IRemoteVideoTrack> videoTrack);
  void getRemoteVideoTrackStats(uid_t uid, RemoteVideoTrackStats& stats);
  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver);
  int unregisterVideoEncodedImageReceiver(IVideoEncodedImageReceiver* receiver);

 private:
  void copyTracks(uid_t uid, bool copyAll, std::set<agora_refptr<rtc::IRemoteVideoTrack>>& tracks);

 private:
  // Track view management
  ViewMap remote_tracks_view_;
  RendererMap remote_tracks_renderer_;
  VideoTrackMap remote_video_track_;
  std::mutex map_mutex_;
};

}  // namespace rtc
}  // namespace agora
