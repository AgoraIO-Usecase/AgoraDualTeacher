#include "util.h"
#include "Shlwapi.h"
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

static std::string GetFileConfig(std::string file, std::string key) {
  if (!PathFileExistsA(file.c_str())) {
    HANDLE handle = CreateFileA(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                                NULL, CREATE_NEW, 0, NULL);
    CloseHandle(handle);
  }

  char data[MAX_PATH] = {0};
  ::GetPrivateProfileStringA(key.c_str(), key.c_str(), NULL, data, MAX_PATH,
                             file.c_str());
  if (strlen(data) == 0) {
    ::WritePrivateProfileStringA(key.c_str(), key.c_str(), "", file.c_str());
  }
  return data;
}

Config GetConfig() {
  char szFilePath[MAX_PATH];
  Config config;
  bool open_config_file = false;
  config.app_id = APP_ID;
  config.customer_id = CUSTOMER_ID;
  config.customer_cerificate_id = CUSTOMER_CERTIFICATE;

  ::GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
  char* lpLastSlash = strrchr(szFilePath, '\\');
  if (lpLastSlash == NULL) return config;
  SIZE_T nNameLen = MAX_PATH - (lpLastSlash - szFilePath + 1);
  strcpy_s(lpLastSlash + 1, nNameLen, "config.ini");

  if (config.app_id == "<enter your agora app id>") {
    config.app_id = GetFileConfig(szFilePath, "APP_ID");
    if (config.app_id.empty()) {
      open_config_file = true;
    }
  }
  if (config.customer_id == "<enter your customer id>") {
    config.customer_id = GetFileConfig(szFilePath, "CUSTOMER_ID");
    if (config.customer_id.empty()) {
      open_config_file = true;
    }
  }
  if (config.customer_cerificate_id == "<enter your customer certificate id>") {
    config.customer_cerificate_id =
        GetFileConfig(szFilePath, "CUSTOMER_CERTIFICATE");
    if (config.customer_cerificate_id.empty()) {
      open_config_file = true;
    }
  }
  if (open_config_file) {
    ::ShellExecuteA(NULL, "open", szFilePath, NULL, NULL, SW_MAXIMIZE);
    exit(1);
  }
  return config;
}
