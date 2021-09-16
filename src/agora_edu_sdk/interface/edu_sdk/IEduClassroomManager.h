//
//  EduClassroomManager.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "EduClassroom.h"
#include "EduMessage.h"
#include "EduUser.h"
#include "IEduAssistantService.h"
#include "IEduStudentService.h"
#include "IEduTeacherService.h"
#include "IEduUserService.h"

namespace agora {
namespace edu {

enum EduClassroomChangeType {
  EDU_CLASSROOM_CHANGE_TYPE_ALL_STUDENTS_CHAT,
  EDU_CLASSROOM_CHANGE_TYPE_COURSE_STATE
};

enum ConnectionState {
  CONNECTION_STATE_DISCONNECTED,
  CONNECTION_STATE_CONNECTING,
  CONNECTION_STATE_CONNECTED,
  CONNECTION_STATE_RECONNECTING,
  CONNECTION_STATE_FULL_ROLE_ABORTED,
  CONNECTION_STATE_ABORTED
};

enum NetworkQuality {
  NETWORK_QUALITY_UNKNOWN,
  NETWORK_QUALITY_HIGH,
  NETWORK_QUALITY_MIDDLE,
  NETWORK_QUALITY_LOW
};

class IEduClassroomEventHandler {
 public:
  // User in or out
  virtual void OnRemoteUsersJoined(
      agora_refptr<IUserInfoCollection> user_info_collection,
      EduClassroom from_classroom) = 0;
  virtual void OnRemoteUsersLeft(
      agora_refptr<IUserEventCollection> user_event_collection,
      EduClassroom from_classroom) = 0;
  virtual void OnRemoteUserStateUpdated(EduUserEvent user_event,
                                        EduUserStateChangeType type,
                                        EduClassroom from_classroom) = 0;

  // message
  virtual void OnRoomChatMessageReceived(
      agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
      EduClassroom from_classroom) = 0;
  virtual void OnRoomCustomMessageReceived(
      agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user,
      EduClassroom from_classroom) = 0;

  // stream
  virtual void OnRemoteStreamsAdded(
      agora_refptr<IStreamInfoCollection> stream_info_collection,
      EduClassroom from_classroom) = 0;
  virtual void OnRemoteStreamsRemoved(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      EduClassroom from_classroom) = 0;
  virtual void OnRemoteStreamUpdated(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      EduClassroom from_classroom) = 0;

  // class room
  virtual void OnClassroomStateUpdated(EduClassroomChangeType type,
                                       EduUser operator_user,
                                       EduClassroom from_classroom) = 0;
  virtual void OnNetworkQualityChanged(NetworkQuality quality, EduUser user,
                                       EduClassroom from_classroom) = 0;

  // property
  virtual void OnClassroomPropertyUpdated(EduClassroom from_classroom,
                                          const char* cause) = 0;
  virtual void OnRemoteUserPropertyUpdated(EduUser user,
                                           EduClassroom from_classroom,
                                           const char* cause) = 0;

  // connection
  virtual void OnConnectionStateChanged(ConnectionState state,
                                        EduClassroom from_classroom) = 0;
  virtual void OnSyncProgress(uint8_t progress) = 0;
};

const char* const kTeacherLimit = "TeacherLimit";
const char* const kAssistantLimit = "AssistantLimit";
const char* const kStudentLimit = "StudentLimit";

struct EduClassroomMediaOptions {
  bool auto_subscribe;
  bool auto_publish;
  char primary_stream_id[kMaxStreamUuidSize];  // 如果用户不填的话，就用user
                                               // uuid当作stream的uuid（去除）

  EduClassroomMediaOptions()
      : auto_subscribe(false), auto_publish(false), primary_stream_id{0} {}
};

struct EduClassroomJoinOptions {
  char user_name[kMaxUserUuidSize];

  EduRoleType role_type;
  EduClassroomMediaOptions media_options;
  bool custom_render;
  bool enable_hwenc;
  bool enable_hwdec;
  EduClassroomJoinOptions()
      : user_name{0},
        custom_render(false),
        enable_hwenc(false),
        enable_hwdec(false),
        role_type(EDU_ROLE_TYPE_INVALID) {}
};

class IEduClassroomManager : public RefCountInterface {
 public:
  // 注意autoPublish为false的时候， 这里不需要做什么。
  // 如果为true的时候，需要调用传递entry publish type。
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 101:communication error:code，透传rtm错误code或者message。
   * 201:media error:code，透传rtc错误code或者message。
   * 301:  network error，透传后台错误msg字段
   */
  virtual EduError JoinClassroom(const EduClassroomJoinOptions& options) = 0;

  /* code:message
   * 1:you haven't joined the room
   */
  virtual agora_refptr<IEduUserService> GetEduUserService() = 0;

  /* code:message
   * 1:you haven't joined the room
   */
  virtual EduClassroom GetClassroomInfo() = 0;

  /* code:message
   * 1:you haven't joined the room
   * 2:parameter XXX is invalid
   */
  virtual size_t GetUserCount(EduRoleType role_type) = 0;

  /* code:message
   * 1:you haven't joined the room
   * 2:parameter XXX is invalid
   */
  virtual agora_refptr<IUserInfoCollection> GetUserList(
      EduRoleType role_type) = 0;

  /* code:message
   * 1:you haven't joined the room
   */
  virtual agora_refptr<IUserInfoCollection> GetFullUserList() = 0;

  /* code:message
   * 1:you haven't joined the room
   */
  virtual agora_refptr<IStreamInfoCollection> GetFullStreamList() = 0;

  /* code:message
   * 1:you haven't joined the room
   */
  virtual EduError LeaveClassroom() = 0;

  virtual void RegisterEventHandler(IEduClassroomEventHandler* handler) = 0;
  virtual void UnregisterEventHandler(IEduClassroomEventHandler* handler) = 0;
};

}  // namespace edu
}  // namespace agora