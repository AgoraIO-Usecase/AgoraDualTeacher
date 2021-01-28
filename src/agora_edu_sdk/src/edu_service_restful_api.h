//
//  edu_service_resful_api.h
//
//  Created by LC on 2020/11/24.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"
#include "base/base_type.h"
#include "base/user_id_manager.h"
#include "edu_service_restful_api.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/rtc_callback.h"
#include "interface/base/EduMessage.h"
#include "interface/edu_sdk/IEduTeacherService.h"
#include "interface/edu_sdk/IEduUserService.h"
#include "internal/IAgoraRteTransferProtocol.h"
#include "rtm/include/IAgoraRtmService.h"
#include "transfer/rest_api_utility.h"
#include "transfer/restful_data_defines.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"

namespace agora {
namespace edu {
class ServiceRestfulHelper
    : public std::enable_shared_from_this<ServiceRestfulHelper> {
 private:
  std::string app_id_;
  std::string scene_uuid_;
  std::string user_uuid_;
  std::string stream_uuid_;
  std::string http_token_;
  std::string auth_;
  utils::worker_type parser_worker_;
  rte::DataRequestType req_;
  utils::RtcAsyncCallback<IEduUserOperationEventHandler>::Type
      edu_user_event_handler_;

 public:
  void Initialize(std::string app_id, std::string scene_uuid,
	  std::string auth,std::string user_uuid, std::string http_token,
                  utils::worker_type parser_worker, rte::DataRequestType req);

  agora::agora_refptr<agora::rte::IDataParam> CreateDataParamCommon(
      const std::string& stream_id);

  int PublishMediaTrack(std::string stream_id, std::string stream_name,
                        const EduStream& stream);

  int PublishMediaTrack(bool publish_audio, bool publish_video,
                        const EduStream& stream);

  int SendPeerMessageToRemoteUser(const char* text, const EduUser& remote_user);

  int SendRoomMessageToAllRemoteUsers(const char* text);

  int SendRoomChatMessage(const char* text);
  int SendPeerChatMessageToRemoteUser(const char* text,
                                      const EduUser& remote_user);

  int UnpublishLocalMediaTrack(const EduStream& stream);

  int UnmuteLocalMediaStream(EduStream stream, bool unmute_video,
                             bool unmute_audio);

  int CreateLocalStream(const EduStream& stream,
                        std::function<void(std::string)>&& cb);

  int MuteLocalMediaStream(EduStream stream, bool mute_video, bool mute_audio);
  int SetRoomProperty(Property property, const char* custom_cause);
  int SetRoomProperties(std::vector<Property> property,
                        const char* custom_cause);

  int SetUserProperty(Property property, const char* custom_cause,
                      EduUser target_user);

  int UpdateCourseState(EduCourseState course_state);
  int AllowAllStudentChat(bool enable);
  int AllowStudentChat(bool enable, EduUser remote_student);

  int CreateOrUpdateStudentStreamWithTeacher(EduStream remote_stream);
  int CreateOrUpdateStudentStreamWithAssistant(EduStream remote_stream);

  int CreateOrUpdateTeacherStream(EduStream remote_stream);

  agora::agora_refptr<agora::rte::IDataParam> CreateDataParamForProperties(
      const std::string& properties_str, bool remove);

  static std::string ConstructPropertiesStr(
      const rte::KeyValPairCollection& properties, const char* json_cause);

  void RegisterEventHandler(IEduUserOperationEventHandler* handler);

  void UnRegisterEventHandler(IEduUserOperationEventHandler* handler);

  utils::RtcAsyncCallback<IEduUserOperationEventHandler>::Type
  GetEventHandler() {
    return edu_user_event_handler_;
  }

 public:
  ServiceRestfulHelper()
      : edu_user_event_handler_(
            utils::RtcAsyncCallback<IEduUserOperationEventHandler>::Create()){};
  ~ServiceRestfulHelper() {
    rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
      if (edu_user_event_handler_) {
        edu_user_event_handler_->Unregister();
        edu_user_event_handler_.reset();
      }
      return ERR_OK;
    });
  }
  ServiceRestfulHelper(const ServiceRestfulHelper&) = default;
};
}  // namespace edu
}  // namespace agora
