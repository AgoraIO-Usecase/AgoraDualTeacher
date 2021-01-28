//
//  edu_user_service_base.cpp
//
//  Created by LC on 2020/11/27.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once
#include "interface/cpp/rte/internal/IAgoraRteTransferProtocol.h"
#include "interface/edu_sdk/IEduClassroomManager.h"
#include "utils/thread/base_worker.h"
namespace agora {
namespace edu {

struct EduUserServiceConfig {
  std::string app_id;
  std::string auth;
  std::string http_token;
  std::string rtc_token;
  utils::worker_type parser_worker;
  rte::DataRequestType req;
  std::string scene_id;
  std::string user_id;
  std::string user_name;
  std::string channel_name;
  std::string stream_uuid;
  EduRoleType role;
  bool is_chat_allowed;
  bool custom_render;
  EduClassroomMediaOptions media_options;
};

class IEduReceiveClassRoomManagerEventHandler {
 public:
  virtual void OnLocalUserStateUpdated(EduUserEvent user_event,
                                       EduUserStateChangeType type) = 0;
  virtual void OnLocalUserPropertyUpdated(EduUser user, const char* cause) = 0;

  virtual void OnLocalStreamChanged(EduStreamEvent stream_event,
                                    MediaStreamState state) = 0;

  virtual void OnRemoteUserStateUpdated(EduUserEvent user_event,
                                        EduUserStateChangeType type) = 0;
  virtual void OnRemoteUserPropertyUpdated(EduUser user, const char* cause) = 0;

  virtual void OnRemoteStreamChanged(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      MediaStreamState state) = 0;

  virtual void OnRemoteStreamAdded(
      agora_refptr<IStreamInfoCollection> stream_event_collection,
      MediaStreamState state) = 0;
};

class IEduUserServiceCallbackHandler {};

}  // namespace edu
}  // namespace agora
