//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraRteScene.h"
#include "IAgoraRteTransferProtocol.h"

namespace agora {
namespace rte {

struct AgoraRteSceneDebugInfo {
  uint64_t cur_sequence_id;
  int cached_sequence_size;
  int fetch_missing_sequence_counter;
  ConnState scene_state;

  AgoraRteSceneDebugInfo()
      : cur_sequence_id(0),
        cached_sequence_size(0),
        fetch_missing_sequence_counter(0),
        scene_state(CONN_STATE_DISCONNECTED) {}
};

class IAgoraRteSceneWithTest : public IAgoraRteScene {
 public:
  virtual void SetConnectionStateChangedForTest(DataReceiverConnState state) = 0;
  virtual void DiscardNextSequenceForTest() = 0;
  virtual void SceneRefresh() = 0;
  virtual void GetSceneDebugInfo(AgoraRteSceneDebugInfo& info) = 0;
};

}  // namespace rte
}  // namespace agora