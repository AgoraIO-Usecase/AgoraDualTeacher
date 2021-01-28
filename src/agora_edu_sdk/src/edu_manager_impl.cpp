//  edu_manager_impl.cpp
//
//  Created by WQX on 2020/11/18.
//  Copyright © 2020 agora. All rights reserved.
//
#include "edu_manager_impl.h"

#include "edu_message_impl.h"

#include "AgoraRefPtr.h"
#include "EduMessage.h"
#include "facilities/tools/api_logger.h"
#include "main/ui_thread.h"
#include "refcountedobject.h"
#include "rte/transfer/rest_api_utility.h"
#include "rte/transfer/transfer_factory.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "edu_classroom_manager_impl.h"

static const char* const MODULE_NAME = "[EduManager]";
static std::atomic_int64_t k_service_cnt = {0};

namespace agora {
namespace edu {

#define AV_RB32(x)                                                      \
  (((uint32_t)((const uint8_t*)(x))[0] << 24) |                         \
   (((const uint8_t*)(x))[1] << 16) | (((const uint8_t*)(x))[2] << 8) | \
   ((const uint8_t*)(x))[3])

#define AV_BASE64_SIZE(x) (((x) + 2) / 3 * 4 + 1)

static char* av_base64_encode(char* out, int out_size, const uint8_t* in,
                              int in_size) {
  static const char b64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char *ret, *dst;
  unsigned i_bits = 0;
  int i_shift = 0;
  int bytes_remaining = in_size;

  if (in_size >= UINT_MAX / 4 || out_size < AV_BASE64_SIZE(in_size))
    return NULL;
  ret = dst = out;
  while (bytes_remaining > 3) {
    i_bits = AV_RB32(in);
    in += 3;
    bytes_remaining -= 3;
    *dst++ = b64[i_bits >> 26];
    *dst++ = b64[(i_bits >> 20) & 0x3F];
    *dst++ = b64[(i_bits >> 14) & 0x3F];
    *dst++ = b64[(i_bits >> 8) & 0x3F];
  }
  i_bits = 0;
  while (bytes_remaining) {
    i_bits = (i_bits << 8) + *in++;
    bytes_remaining--;
    i_shift += 8;
  }
  while (i_shift > 0) {
    *dst++ = b64[(i_bits << 6 >> i_shift) & 0x3f];
    i_shift -= 6;
  }
  while ((dst - ret) & 3) *dst++ = '=';
  *dst = '\0';
  return ret;
}

static std::string base64Encoder(std::string src) {
  std::string res(AV_BASE64_SIZE(src.length()),' ');
  av_base64_encode(res.data(), AV_BASE64_SIZE(src.length()),
                   (const uint8_t*)src.data(), src.length());
  
  return res;
}

EduManager::EduManager()
    : event_handlers_(
          utils::RtcAsyncCallback<IEduManagerEventHandler>::Create()) {}

EduManager::~EduManager() {
  ASSERT_IS_UI_THREAD();

  if (service_) {
    if (0 == k_service_cnt) {
      LOG_ERR_AND_RET("service count already zero, shouldn't be");
    }

    if (--k_service_cnt == 0) {
      service_->Release();
      service_ = nullptr;
    }
  }
}

int EduManager::Initialize(const EduConfiguration& config) {
  API_LOGGER_MEMBER(
      "EduConfiguration: (appid: %s, customer_id: %s, customer_certificate: "
      "%s, "
      "user_uuid: %s, tag: %d, log_level: %d, "
      "log_directory_path: %s, ",
      LITE_STR_CONVERT(config.app_id), LITE_STR_CONVERT(config.customer_id),
      LITE_STR_CONVERT(config.customer_certificate),
      LITE_STR_CONVERT(config.user_uuid), config.tag, config.log_level,
      LITE_STR_CONVERT(config.log_file_path));

  if (!config.app_id || !config.customer_id || !config.customer_certificate) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT,
                        "no APP ID or customer ID or certificate specified.");
  }

  if (!config.user_uuid) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "no user_id specified.");
  }

  edu_program_info_.app_id = config.app_id;
  edu_program_info_.customer_id = config.customer_id;
  edu_program_info_.customer_certificate = config.customer_certificate;
  std::string raw_auth = config.customer_id;
  raw_auth += ":";
  raw_auth += config.customer_certificate;

  edu_program_info_.auth = "Basic " + base64Encoder(raw_auth);

  edu_user_info_.user_uuid = config.user_uuid;

  // Use default config
  data_transfer_method_;

  if (!service_) {
    service_ = AgoraEduService::Create();
    ++k_service_cnt;
  }

  if (!service_) {
    --k_service_cnt;
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create Agora Edu service.");
  }

  AgoraEduServiceConfiguration service_config;
  service_config.log_file_path = config.log_file_path;

  if (service_->Initialize(service_config) != ERR_OK) {
    service_->Release();
    service_ = nullptr;
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to initialize Agora Edu service.");
  }

  if (config.log_file_path && config.log_file_size > 0) {
    commons::set_log_file(config.log_file_path, config.log_file_size);
  }

  commons::set_log_filters(commons::AgoraLogger::ApiLevelToUtilLevel(
      static_cast<commons::LOG_LEVEL>(config.log_level)));

  auto param = rte::CreateDataParam();

  param->AddString(PARAM_APP_ID, edu_program_info_.app_id);
  param->AddString(PARAM_AUTH, edu_program_info_.auth);
  param->AddString(PARAM_USER_UUID, edu_user_info_.user_uuid);

  auto param_temp = new rte::DataTransferMethod;
  if (param_temp) {
    *param_temp = data_transfer_method_;
    param->AddPtr(PARAM_OBJECT_PTR2, param_temp);
  }

  data_receiver_ = rte::TransferFactory::CreateRteDataReceiver(
      data_transfer_method_.data_receiver_type);

  if (!data_receiver_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create RTE data receiver.");
  }

  int err = data_receiver_->SetParam(param);
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err, "failed to set param through RTE data receiver.");
  }

  data_receiver_->RegisterEventHandler(this);

  err = data_receiver_->Login();
  if (err != ERR_OK) {
    LOG_ERR_AND_RET_INT(err, "failed to login to RTE data receiver.");
  }

  return ERR_OK;
}

void EduManager::Release() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    if (data_receiver_) {
      data_receiver_->UnregisterEventHandler(this);
      data_receiver_->Logout();
      data_receiver_ = nullptr;
    }

    delete this;
    return ERR_OK;
  });

  if (k_service_cnt == 0) {
    utils::UninitializeUtils();
  }
}

agora_refptr<IEduClassroomManager> EduManager::CreateClassroomManager(
    const EduClassroomConfig& config) {
  API_LOGGER_MEMBER("config(room_uuid: %s, class_type: %d)", config.room_uuid,
                    config.class_type);

  agora_refptr<IEduClassroomManager> edu_classroom_manager;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (!rte_data_receiver_login_succeeded_) {
      LOG_ERR_AND_RET_INT(ERR_NOT_READY, "RTE data receiver login failed");
    }

    if (utils::IsNullOrEmpty(config.room_uuid)) return ERR_OK_;

    edu_classroom_manager = new RefCountedObject<EduClassroomManager>(
        config, edu_program_info_, data_receiver_, data_transfer_method_,
        edu_user_info_);
    return ERR_OK_;
  });

  return edu_classroom_manager;
}

void EduManager::LogMessage(const char* message, LogLevel level) {
  API_LOGGER_MEMBER("message: %s, level: %d", LITE_STR_CONVERT(message), level);

  if (utils::IsNullOrEmpty(message)) {
    LOG_ERR_AND_RET("invalid message in LogMessage()");
  }

  commons::log(static_cast<commons::log_filters>(
                   commons::AgoraLogger::ApiLevelToUtilLevel(
                       static_cast<commons::LOG_LEVEL>(level))),
               message);
}

void EduManager::UploadDebugItem(DebugItem item) {
  // TODO(WQX) implement later
}

void EduManager::OnLoginSuccess(const std::string& user_token) {
  API_LOGGER_CALLBACK(OnLoginSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    rte_data_receiver_login_succeeded_ = true;
    edu_user_info_.user_token = user_token;

    event_handlers_->Post(LOCATION_HERE, [](auto event_handler) {
      event_handler->OnInitializeSuccess();
    });
    return ERR_OK;
  });
}

void EduManager::OnLoginFailure() {
  API_LOGGER_CALLBACK(OnLoginFailure, nullptr);

  LOG_ERR("RteEngine::OnLoginFailure!!!");

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    rte_data_receiver_login_succeeded_ = false;

    event_handlers_->Post(LOCATION_HERE, [](auto event_handler) {
      event_handler->OnInitializeFailed();
    });
    return ERR_OK;
  });
}
void EduManager::OnConnectionStateChanged(rte::DataReceiverConnState state) {
  API_LOGGER_CALLBACK(OnConnectionStateChanged, "state: %d", state);

  // TODO(WQX) don't need to implement currently
}

void EduManager::OnMessageReceivedFromPeer(const std::string& peer_id,
                                           const std::string& message) {
  API_LOGGER_CALLBACK(OnMessageReceivedFromPeer, "peer_id: %s, message: %s",
                      peer_id.c_str(), message.c_str());

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    commons::cjson::JsonWrapper root;
    root.parse(message.c_str());
    if (!root.isObject()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to get JSON root in "
                          "OnMessageReceivedFromPeer():\n----\n%s\n----\n",
                          message.c_str());
    }

    commons::cjson::JsonWrapper dict_data;
    rte::RtmResponseBase base;
    if (!rte::RestfulDataParser::RtmParseResponseBase(root, base, dict_data,
                                                      false) ||
        !dict_data.isValid()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to get JSON root in "
                          "OnMessageReceivedFromPeer():\n----\n%s\n----\n",
                          message.c_str());
    }

    auto parse_data = std::make_unique<rte::RtmPeerMessage>();
    *static_cast<rte::RtmResponseBase*>(parse_data.get()) = base;
    if (rte::RestfulDataParser::RtmParsePeerMessage(dict_data, *parse_data)) {
      auto msg_data = std::move(*parse_data);
      agora_refptr<IAgoraEduMessage> send_msg =
          new RefCountedObject<AgoraEduMessage>();
      send_msg->SetEduMessage(msg_data.msg.c_str());
      send_msg->SetTimestamp(msg_data.ts);
      EduBaseUser user_info;
      strncpy(user_info.user_name, msg_data.from_user.user_name.c_str(),
              kMaxUserUuidSize - 1);
      strncpy(user_info.user_uuid, msg_data.from_user.user_uuid.c_str(),
              kMaxUserUuidSize - 1);
      user_info.role = RoleString2EduRoleType(msg_data.from_user.role);
      switch (parse_data->cmd) {
        case rte::RTM_CMD_USER_MESSAGE: {
          event_handlers_->Post(
              LOCATION_HERE, [send_msg, user_info](auto event_handler) {
                event_handler->OnPeerMessageReceived(send_msg, user_info);
              });
        } break;
        case rte::RTM_CMD_CUSTOM_MESSAGE: {
          event_handlers_->Post(
              LOCATION_HERE, [send_msg, user_info](auto event_handler) {
                event_handler->OnPeerCustomMessageReceived(send_msg, user_info);
              });
        } break;
        default:
          LOG_INFO("OnMessageReceivedFromPeer unknown cmd: %d",
                   parse_data->cmd);
          break;
      }
    }

    return ERR_OK_;
  });
}

void EduManager::RegisterEventHandler(IEduManagerEventHandler* handler) {
  API_LOGGER_MEMBER("handler: %p", handler);

  if (!handler) {
    LOG_ERR_AND_RET("nullptr handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(handler);
    return ERR_OK;
  });
}

void EduManager::UnregisterEventHandler(IEduManagerEventHandler* handler) {
  API_LOGGER_MEMBER("handler: %p", handler);

  if (!handler) {
    LOG_ERR_AND_RET("nullptr handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(handler);
    return ERR_OK;
  });
}

EduRoleType RoleString2EduRoleType(const std::string& role_string) {
  if (role_string.compare("administrator") == 0) {
    return edu::EDU_ROLE_TYPE_ASSISTANT;
  } else if (role_string.compare("host") == 0) {
    return edu::EDU_ROLE_TYPE_TEACHER;
  } else if (role_string.compare("audience") == 0) {
    return edu::EDU_ROLE_TYPE_STUDENT;
  } else {
    return edu::EDU_ROLE_TYPE_INVALID;
  }
}  // namespace edu

std::string EduRoleType2RoleString(const EduRoleType& role_type) {
  switch (role_type) {
    case edu::EDU_ROLE_TYPE_ASSISTANT:
      return "administrator";
    case edu::EDU_ROLE_TYPE_TEACHER:
      return "host";
    case edu::EDU_ROLE_TYPE_STUDENT:
      return "audience";
    default:
      return "";
  }
}

AGORA_API IEduManager* AGORA_CALL createAgoraEduManager() {
  return new EduManager;
}

AGORA_API const char* AGORA_CALL getEduVersion() { return EDU_SDK_VERSION; }

}  // namespace edu
}  // namespace agora
