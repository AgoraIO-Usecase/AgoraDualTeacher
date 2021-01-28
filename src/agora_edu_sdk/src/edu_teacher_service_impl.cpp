//
//  edu_teacher_service_impl.cpp
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#include "edu_teacher_service_impl.h"
#include <ctime>
#include "edu_user_service_impl.h"
#include "refcountedobject.h"
#define ROOM_KEY_STATE_COURSE_STATE "course_state"
#define ROOM_KEY_COURSE_START_TIME "course_start_time"
#define ROOM_KEY_MUTE_ALL_CHAT "is_student_chat_allowed"
#define ROOM_KEY_MUTE_CHAT "is_chat_allowed"

namespace agora {
namespace edu {

static std::string get_current_time() {
  time_t now = time(0);
  tm* gmtm = gmtime(&now);
  return asctime(gmtm);
}

EduError EduTeacherService::UpdateCourseState(EduCourseState course_state) {
  Property kv_course_state;
  strcpy_s(kv_course_state.key, ROOM_KEY_STATE_COURSE_STATE);
  if (course_state == EDU_COURSE_STATE_START) {
    *kv_course_state.value = '1';
  } else {
    *kv_course_state.value = '0';
  }
  kv_course_state.value[1] = 0;
  Property kv_course_start_time;
  strcpy_s(kv_course_start_time.key, ROOM_KEY_COURSE_START_TIME);
  strcpy_s(kv_course_start_time.value, get_current_time().c_str());
  std::vector<Property> vec;
  vec.emplace_back(kv_course_state);
  vec.emplace_back(kv_course_start_time);
  int err = base->rest_api_helper_->SetRoomProperties(vec, "UpdateCourseState");
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:UpdateCourseState failed");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

EduError EduTeacherService::AllowAllStudentChat(bool enable) {
  Property kv;
  strcpy(kv.key, ROOM_KEY_MUTE_ALL_CHAT);
  *kv.value = enable + '0';
  return SetRoomProperty(kv, "");
}

EduError EduTeacherService::AllowStudentChat(bool enable,
                                             EduUser remote_student) {
  Property kv;
  strcpy(kv.key, ROOM_KEY_MUTE_CHAT);
  *kv.value = enable + '0';
  return SetUserProperty(kv, "", remote_student);
}

EduError EduTeacherService::StartShareScreen(const EduShareScreenConfig& config,
                                             EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=,&err,&stream] {
    err = base->rtc_manager_->StartShareScreen(config, stream);
    return ERR_OK;
	  });
  if (err) return EDU_ERROR_RTC(err, "rtc:StartShareScreen failed");
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
EduError EduTeacherService::StopShareScreen(EduStream& stream) {
  int err = ERR_OK;
  rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err, &stream] {
    err = base->rtc_manager_->StopShareScreen(stream);
    return ERR_OK;
  });
  if (err) return EDU_ERROR_RTC(err, "rtc:StopShareScreen failed");
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
EduError EduTeacherService::SetCustomRender(bool enabled) {
  return base->SetCustomRender(enabled);
}
EduError EduTeacherService::CreateOrUpdateStudentStream(
    EduStream remote_stream) {
  int err = base->rest_api_helper_->CreateOrUpdateStudentStreamWithTeacher(
      remote_stream);
  if (err)
    return EDU_ERROR_NETWORK(err, "net:CreateOrUpdateStudentStream failed!");
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
void EduTeacherService::RegisterOperationEventHandler(
    IEduTeacherOperationEventHandler* handler) {
  base->RegisterOperationEventHandler(handler);
}
void EduTeacherService::UnregisterOperationEventHandler(
    IEduTeacherOperationEventHandler* handler) {
  base->UnregisterOperationEventHandler(handler);
}

EduLocalUser EduTeacherService::GetLocalUserInfo() {
  return base->GetLocalUserInfo();
}

agora_refptr<IStreamInfoCollection> EduTeacherService::GetLocalStreams() {
  return base->GetLocalStreams();
}

EduTeacherService::EduTeacherService(const EduUserServiceConfig& config) {
  base = new RefCountedObject<EduUserService>(config);
}

EduTeacherService::~EduTeacherService() {}

EduError EduTeacherService::SetVideoConfig(const EduStream& stream,
                                           const EduVideoConfig& config) {
  return base->SetVideoConfig(stream, config);
}

// media
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduTeacherService::CreateLocalStream(EduStreamConfig config,
                                                     EduStream& stream) {
  return base->CreateLocalStream(config, stream);
}

/* code:message
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduTeacherService::SwitchCamera(const EduStream& stream,
                                         const char* device_id) {
  return base->SwitchCamera(stream, device_id);
}

// stream
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduTeacherService::SubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->SubscribeStream(stream, options);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduTeacherService::UnsubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->UnsubscribeStream(stream, options);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::PublishStream(const EduStream& stream) {
  return base->PublishStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::UnpublishStream(const EduStream& stream) {
  return base->UnpublishStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::MuteStream(const EduStream& stream) {
  return base->MuteStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::UnmuteStream(const EduStream& stream) {
  return base->UnmuteStream(stream);
}

// message
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SendRoomCustomMessage(const char* text) {
  return base->SendRoomCustomMessage(text);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SendUserCustomMessage(const char* text,
                                            const EduUser& remote_user) {
  return base->SendUserCustomMessage(text, remote_user);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SendRoomChatMessage(const char* text) {
  return base->SendRoomChatMessage(text);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SendUserChatMessage(const char* text,
                                                const EduUser& remote_user) {
  return base->SendUserChatMessage(text, remote_user);
}

// process action
// 一期教育SDK没有这个方法，只是给娱乐使用
EduError EduTeacherService::StartActionWithConfig(
    const EduStartActionConfig& config) {
  return base->StartActionWithConfig(config);
}
EduError EduTeacherService::StopActionWithConfig(
    const EduStopActionConfig& config) {
  return base->StopActionWithConfig(config);
}

// property
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SetRoomProperty(Property property,
                                            const char* custom_cause) {
  return base->SetRoomProperty(property, custom_cause);
}
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduTeacherService::SetUserProperty(Property property,
                                            const char* custom_cause,
                                            EduUser target_user) {
  return base->SetUserProperty(property, custom_cause, target_user);
}

// render
/* code:message
 * 1:parameter XXX is invalid
 */
EduError EduTeacherService::SetStreamView(EduStream stream, View* view) {
  return base->SetStreamView(stream, view);
}
EduError EduTeacherService::SetStreamView(EduStream stream, View* view,
                                          const EduRenderConfig& config) {
  return base->SetStreamView(stream, view, config);
}

void EduTeacherService::RegisterEventHandler(
    IEduUserEventHandler* event_handler) {
  return base->RegisterEventHandler(event_handler);
}
void EduTeacherService::UnregisterEventHandler(
    IEduUserEventHandler* event_hadler) {
  return base->UnregisterEventHandler(event_hadler);
}
void EduTeacherService::RegisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  return base->RegisterOperationEventHandler(handler);
}
void EduTeacherService::UnregisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  return base->UnregisterOperationEventHandler(handler);
}
void EduTeacherService::Destory() { return base->Destory(); }

void EduTeacherService::OnLocalUserStateUpdated(EduUserEvent user_event,
                                                EduUserStateChangeType type) {
  base->OnLocalUserStateUpdated(user_event, type);
}

void EduTeacherService::OnLocalUserPropertyUpdated(EduUser user,
                                                   const char* cause) {
  base->OnLocalUserPropertyUpdated(user, cause);
}

void EduTeacherService::OnLocalStreamChanged(EduStreamEvent stream_event,
                                             MediaStreamState state) {
  base->OnLocalStreamChanged(stream_event, state);
}

void EduTeacherService::OnRemoteUserStateUpdated(EduUserEvent user_event,
                                                 EduUserStateChangeType type) {
  base->OnRemoteUserStateUpdated(user_event, type);
}

void EduTeacherService::OnRemoteUserPropertyUpdated(EduUser user,
                                                    const char* cause) {
  base->OnRemoteUserPropertyUpdated(user, cause);
}

void EduTeacherService::OnRemoteStreamChanged(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamChanged(stream_event_collection, state);
}

void EduTeacherService::OnRemoteStreamAdded(
    agora_refptr<IStreamInfoCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamAdded(stream_event_collection, state);
}

#if 0
void test() {
  agora_refptr<EduTeacherService> ptr =
      new RefCountedObject<EduTeacherService>();
}
#endif

}  // namespace edu
}  // namespace agora