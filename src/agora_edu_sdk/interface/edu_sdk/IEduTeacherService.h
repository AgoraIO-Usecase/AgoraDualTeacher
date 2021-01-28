//
//  EduTeacherService.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once
#include <windows.h>
#include "EduClassroom.h"
#include "IEduUserService.h"

namespace agora {
namespace edu {

struct EduShareScreenConfig {
  char stream_uuid[kMaxStreamUuidSize];
  char stream_name[kMaxStreamUuidSize];

  // Align RTC SDK
  // 对齐RTC SDK， 比如可以设置是那个应用
  HWND hwnd;
  RECT rc;
  bool enableRect;
  int bitrate;
  int fps;

  EduShareScreenConfig()
      : stream_uuid{0},
        stream_name{0},
        hwnd(nullptr),
        enableRect(false),
        bitrate(0),
        fps(10) {}
};

class IEduTeacherOperationEventHandler : public IEduUserOperationEventHandler {
 public:
  virtual void OnCourseStateUpdated(EduCourseState current_state,
                                    EduError err) = 0;

  virtual void OnAllStudentChaAllowed(bool current_enable, EduError err) = 0;
  virtual void OnStudentChatAllowed(bool current_enable, EduError err) = 0;

  virtual void OnCreateOrUpdateStudentStreamCompleted(EduStream stream,
                                                      EduError err) = 0;
};

class IEduTeacherService : public IEduUserService {
 public:
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError UpdateCourseState(EduCourseState course_state) = 0;

  // chat
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError AllowAllStudentChat(bool enable) = 0;
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError AllowStudentChat(bool enable, EduUser remote_student) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError StartShareScreen(const EduShareScreenConfig& config,
                                    EduStream& stream) = 0;
  /* code:message
   * 1:parameter XXX is invalid
   * 2:internal error：可以内部订阅具体什么错误
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError StopShareScreen(EduStream& stream) = 0;

  // Student Stream
  virtual EduError CreateOrUpdateStudentStream(EduStream remote_stream) = 0;

  virtual void RegisterOperationEventHandler(
      IEduTeacherOperationEventHandler* handler) = 0;
  virtual void UnregisterOperationEventHandler(
      IEduTeacherOperationEventHandler* handler) = 0;
};

}  // namespace edu
}  // namespace agora