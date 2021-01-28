#pragma once
#include <stdlib.h>  //头文件包含rand和srand函数
#include <time.h>
#include <QCoreApplication>
#include <QEvent>
#include <QString>
#include <functional>
#include <memory>
#include <string>

#include "EduStream.h"
#include "IEduClassroomManager.h"

using namespace agora::edu;

#define MAX_STREAM_COUNT 3

enum StreamUuidType {
  TEACHER_FIRST_STREAM_UUID,
  TEACHER_SECOND_STREAM_UUID,
  STUDENT_FIRST_STREAM_UUID,
  RAND_STREAM_UUID
};

enum RoomMessageType {
  ROOM_ENTER_READY, SWITCH_WIDGET, KICK_MSG };

#define APP_ID ""
#define CUSTOMER_ID ""
#define CUSTOMER_CERTIFICATE ""

#define RAISE_KEY "Raise"
#define RAISE_TRUE "True"
#define RAISE_FALSE "False"

class AgoraVideoWidget;
using UuidWidgetPair = std::pair<std::string, AgoraVideoWidget*>;

#define SET_RANDOM_STR(str)                        \
  str = std::to_string(GetRandom(100, RAND_MAX)) + \
        std::to_string(GetRandom(100, RAND_MAX));

#define SET_RANDOM_NUMBER(number) \
  { number = (GetRandom(100, RAND_MAX) << 16) | GetRandom(100, RAND_MAX); }

#define ENUM_CASE_RETURN_STR(nEnum) \
  case nEnum:                       \
    return #nEnum;

#define AGORA_EVENT (QEvent::User + 0x01)

#define STR(str) #str

enum ClassroomStateChangeType {
  CLASSROOM_STATE_CHANGE_TYPE_MUTE,
  CLASSROOM_STATE_CHANGE_TYPE_CHAT,
  CLASSROOM_STATE_CHANGE_TYPE_RAISE
};

int GetRandom(int min, int max);

QString str2qstr(const std::string str);

std::string qstr2str(const QString qstr);

const char* connection_state_to_str(ConnectionState type);

void SetInitStreamUuidFlag(bool init);

EduStreamConfig CreateRandUuidStreamConfig(const StreamUuidType& index,
                                           std::string stream_name,
                                           EduVideoSourceType source_type,
                                           bool has_video, bool has_audio);

class AgoraEvent : public QEvent {
  const static Type TYPE = static_cast<Type>(AGORA_EVENT);

 public:
  AgoraEvent() : QEvent(TYPE){};

  static void PostAgoraEvent(AgoraEvent* event, QObject* obj,
                             const std::function<void()>& task) {
    event->SetTask(task);
    QCoreApplication::postEvent(obj, event);
  }

  void SetTask(const std::function<void()>& task) { task_ = task; }
  void RunTask() { task_(); }

 private:
  std::function<void()> task_;
};
