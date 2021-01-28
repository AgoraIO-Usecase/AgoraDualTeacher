#include "util.h"

static bool g_initStreamUuid = false;
int g_eduStreamUuid[MAX_STREAM_COUNT];

int GetRandom(int min, int max) {
  static thread_local bool init = false;
  if (!init) {
    srand(time(0));
    init = true;
  }
  return (min + rand() % (max - min + 1));
}

QString str2qstr(const std::string str) {
  return QString::fromLocal8Bit(str.data());
}

std::string qstr2str(const QString qstr) {
  QByteArray cdata = qstr.toLocal8Bit();
  return std::string(cdata);
}

const char* connection_state_to_str(ConnectionState type) {
  switch (type) {
    ENUM_CASE_RETURN_STR(ConnectionState::CONNECTION_STATE_DISCONNECTED)
    ENUM_CASE_RETURN_STR(ConnectionState::CONNECTION_STATE_CONNECTING)
    ENUM_CASE_RETURN_STR(ConnectionState::CONNECTION_STATE_CONNECTED)
    ENUM_CASE_RETURN_STR(ConnectionState::CONNECTION_STATE_RECONNECTING)
    ENUM_CASE_RETURN_STR(ConnectionState::CONNECTION_STATE_ABORTED)
    default:
      return "CONNECTION_STATE_UNKNOWN";
  }
}

void SetInitStreamUuidFlag(bool init) { g_initStreamUuid = init; }

EduStreamConfig CreateRandUuidStreamConfig(const StreamUuidType& index,
                                           std::string stream_name,
                                           EduVideoSourceType source_type,
                                           bool has_video, bool has_audio) {
  if (!g_initStreamUuid) {
    // edu stream with same user uuid will be kept in rtm even if leave
    // classroom,so use global stream uuid
    SET_RANDOM_NUMBER(g_eduStreamUuid[0]);
    SET_RANDOM_NUMBER(g_eduStreamUuid[1]);
    SET_RANDOM_NUMBER(g_eduStreamUuid[2]);
    g_initStreamUuid = true;
  }

  EduStreamConfig config;
  int uuid;
  if (index > 2) {
    SET_RANDOM_NUMBER(uuid);
  } else {
    uuid = g_eduStreamUuid[index];
  }

  config.stream_uuid = uuid;
  strncpy(config.stream_name, stream_name.c_str(), kMaxStreamUuidSize);
  config.enable_camera = has_video;
  config.enable_microphone = has_audio;
  config.video_soruce_type = source_type;
  return config;
}
