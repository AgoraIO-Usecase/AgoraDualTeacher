//
//  EduManager.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "AgoraRefPtr.h"
#include "EduBaseTypes.h"
#include "EduMessage.h"
#include "EduStream.h"
#include "EduUser.h"

namespace agora {
namespace edu {

class IEduClassroomManager;

enum LogLevel {
  LOG_LEVEL_NONE = 0x0000,
  LOG_LEVEL_INFO = 0x0001,
  LOG_LEVEL_WARN = 0x0002,
  LOG_LEVEL_ERROR = 0x0004,
  LOG_LEVEL_FATAL = 0x0008,
};

enum DebugItem { DEBUG_LOG };

struct EduActionMessage {
  const char* process_uuid;
  EduActionType action;
  long timeout;
  EduBaseUser from_user;

  Property* payload;
  size_t payload_count;
};

struct EduConfiguration {
  AppId app_id;
  CustomerId customer_id;
  CustomerCertificate customer_certificate;

  char user_uuid[kMaxUserUuidSize];

  // 用于数据统计
  uint8_t tag;

  LogLevel log_level;
  const char* log_file_path;
  size_t log_file_size;

  EduConfiguration()
      : app_id(nullptr),
        customer_id(nullptr),
        customer_certificate(nullptr),
        user_uuid{0},
        tag(0),
        log_level(LOG_LEVEL_NONE),
        log_file_path(nullptr) {}
};

struct EduClassroomConfig {
  char room_uuid[kMaxRoomUuidSize];
  char room_name[kMaxRoomUuidSize];

  EduClassroomType class_type;
  EduClassroomConfig() : room_uuid{0}, room_name{0} {}
};

class IEduManagerEventHandler {
 public:
  virtual void OnInitializeSuccess() = 0;
  virtual void OnInitializeFailed() = 0;

  virtual void OnDebugItemUploadSuccess(const char* upload_serial_number) = 0;
  virtual void OnDebugItemUploadFailure(EduError err) = 0;

  // message
  virtual void OnPeerMessageReceived(
      agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user) = 0;
  virtual void OnPeerCustomMessageReceived(
      agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user) = 0;

  // 一期教育SDK没有这个方法，只是给娱乐使用
  virtual void OnUserActionMessageReceived(EduActionMessage action_message) = 0;
};

// 0代表正常
// 1-10 本地错误code
// 101 RTM错误-通信错误
// 201 RTC错误-媒体错误
// 301 HTTP错误-网络错误

class IEduManager {
 public:
  // 初始EduManager，里面包含创建EduManager 和 登录rtm
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 101:communication error（code），透传rtm错误code。
   * 301:network error（code），透传后台错误msg字段
   */
  // /apps/{appId}/v1/users/{userUuid}/login
  virtual int Initialize(const EduConfiguration& config) = 0;

  virtual void Release() = 0;

  // 生成本地 EduClassroomManager对象
  virtual agora_refptr<IEduClassroomManager> CreateClassroomManager(
      const EduClassroomConfig& config) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   */
  virtual void LogMessage(const char* message, LogLevel level) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 301:network error，透传后台错误msg字段
   */
  virtual void UploadDebugItem(DebugItem item) = 0;

  virtual void RegisterEventHandler(IEduManagerEventHandler* handler) = 0;
  virtual void UnregisterEventHandler(IEduManagerEventHandler* handler) = 0;

 protected:
  virtual ~IEduManager() {}
};

// create IEduManager object
AGORA_API agora::edu::IEduManager* AGORA_CALL createAgoraEduManager();

// 返回SDK版本， 目前是RTC的SDKversion + .101
AGORA_API const char* AGORA_CALL getEduVersion();

}  // namespace edu
}  // namespace agora
