//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "rte_engine.h"

#include <atomic>

#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif  // HAS_BUILTIN_EXTENSIONS

//#include "core/agora_service_impl.h"
#include "facilities/tools/api_logger.h"
//#include "media/media_control.h"
#include "rte_scene.h"
#include "transfer/transfer_factory.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "agora_servcie.h"

static const char* const MODULE_NAME = "[RTE.RE]";

// currently RTE engine will be the only one that multiple instances using the same Agora Service
// instance, so have to use reference count
static std::atomic_int64_t k_service_cnt = {0};

namespace agora {
namespace rte {

RteEngine::RteEngine() : event_handlers_(utils::RtcAsyncCallback<IRteEventHandler>::Create()) {}

RteEngine::~RteEngine() {
  ASSERT_IS_UI_THREAD();

  if (service_) {
    if (0 == k_service_cnt) {
      LOG_ERR_AND_RET("service count already zero, shouldn't be");
    }

    if (--k_service_cnt == 0) {
      service_->release();
      service_ = nullptr;
    }
  }
}

int RteEngine::InitializeEx(const EngagementConfigurationEx& config) {
  data_transfer_method_ = config.data_transfer_method;
  return Initialize(config);
}

int RteEngine::Initialize(const EngagementConfiguration& config) {
  API_LOGGER_MEMBER(
      "RTE engine config: (appid_or_token: %s, user_id: %s, user_name: %s, "
      "scene_preset: %d, context: %p, log_file_path: %s, log_level: %d, log_file_size: %u, "
      "event_handler: %p)",
      LITE_STR_CONVERT(config.appid_or_token), LITE_STR_CONVERT(config.user_id),
      LITE_STR_CONVERT(config.user_name), config.scene_preset, config.context,
      LITE_STR_CONVERT(config.log_file_path), config.log_level, config.log_file_size,
      config.event_handler);

  if (service_) {
    return ERR_OK;
  }

  if (!config.appid_or_token) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "no APP ID or token specified.");
  }

  if (!config.user_id) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "no user_id specified.");
  }

  rte_user_info_.user_id = config.user_id;
  rte_user_info_.user_name = LITE_STR_CAST(config.user_name);

  appid_or_token_ = config.appid_or_token;

  service_ = createAgoraService();
  if (!service_) {
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create Agora service.");
  }
  ++k_service_cnt;

  base::AgoraServiceConfigEx svc_cfg_ex;
  svc_cfg_ex.appId = appid_or_token_.c_str();
  svc_cfg_ex.context = config.context;
  svc_cfg_ex.useStringUid = true;
  svc_cfg_ex.enableVideo = true;

  auto service_ex = static_cast<base::IAgoraServiceEx*>(service_);

  if (service_ex->initializeEx(svc_cfg_ex) != ERR_OK) {
    service_->release();
    service_ = nullptr;
    LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to initialize Agora service.");
  }

  // set log path, size and filters
  if (config.log_file_path && config.log_file_size > 0) {
    commons::set_log_file(config.log_file_path, config.log_file_size);
  }

  commons::set_log_filters(commons::AgoraLogger::ApiLevelToUtilLevel(config.log_level));

#if defined(HAS_BUILTIN_EXTENSIONS)
  auto ext_provider = rtc::ExtensionProviderBuiltin::Create();
  auto ext_ctrl = service_ex->getExtensionControl();
  if (ext_ctrl) {
    ext_ctrl->registerExtensionProvider(BUILTIN_EXTENSION_PROVIDER, ext_provider);
  }
#endif  // HAS_BUILTIN_EXTENSIONS

  // Async login to IRteDataReceiver
  auto param = CreateDataParam();
  param->AddString(PARAM_APP_ID, appid_or_token_);
  param->AddString(PARAM_USER_UUID, config.user_id);
  param->AddPtr(PARAM_OBJECT_PTR, service_);

  DataTransferMethod* param_temp = new DataTransferMethod;
  if (param_temp) {
    *param_temp = data_transfer_method_;
    param->AddPtr(PARAM_OBJECT_PTR2, param_temp);
  }
  data_receiver_ = TransferFactory::CreateRteDataReceiver(data_transfer_method_.data_receiver_type);
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

void RteEngine::Release() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    if (data_receiver_) {
      data_receiver_->UnregisterEventHandler(this);
      data_receiver_->Logout();
      data_receiver_ = nullptr;
    }

    delete this;
    return ERR_OK;
  });
}

agora_refptr<IAgoraRteScene> RteEngine::CreateAgoraRteScene(
    const SceneConfiguration& scene_config) {
  API_LOGGER_MEMBER("Scene config: (scene_uuid: %s)", scene_config.scene_uuid);

  agora_refptr<IAgoraRteScene> rte_scene;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&]() {
    if (!rte_data_receiver_login_succeeded_) {
      LOG_ERR_AND_RET_INT(ERR_NOT_READY, "RTE data receiver login failed");
    }

    rte_scene =
        new RefCountedObject<RteScene>(this, service_, scene_config, appid_or_token_.c_str(),
                                       data_receiver_, data_transfer_method_, rte_user_info_);
    return ERR_OK_;
  });

  return rte_scene;
}

void RteEngine::RegisterEventHandler(IRteEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RteEngine::UnregisterEventHandler(IRteEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

void RteEngine::OnLoginSuccess(const std::string& user_token) {
  API_LOGGER_CALLBACK(OnLoginSuccess, nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this, user_token] {
    rte_data_receiver_login_succeeded_ = true;
    rte_user_info_.user_token = user_token;

    event_handlers_->Post(LOCATION_HERE,
                          [](auto event_handler) { event_handler->OnInitializeSuccess(); });
    return ERR_OK;
  });
}

void RteEngine::OnLoginFailure() {
  API_LOGGER_CALLBACK(OnLoginFailure, nullptr);

  LOG_ERR("RteEngine::OnLoginFailure!!!");

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    rte_data_receiver_login_succeeded_ = false;

    event_handlers_->Post(LOCATION_HERE,
                          [](auto event_handler) { event_handler->OnInitializeFailed(); });
    return ERR_OK;
  });
}

void RteEngine::OnConnectionStateChanged(DataReceiverConnState state) {
  API_LOGGER_CALLBACK(OnConnectionStateChanged, "state: %d", state);

  // TODO(jxm): What to do?
}

void RteEngine::OnMessageReceivedFromPeer(const std::string& peer_id, const std::string& message) {
  API_LOGGER_CALLBACK(OnMessageReceivedFromPeer, "peer_id: %s, message: %s", peer_id.c_str(),
                      message.c_str());

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    commons::cjson::JsonWrapper root;
    root.parse(message.c_str());
    if (!root.isObject()) {
      LOG_ERR_AND_RET_INT(
          ERR_FAILED, "failed to get JSON root in OnMessageReceivedFromPeer():\n----\n%s\n----\n",
          message.c_str());
    }

    commons::cjson::JsonWrapper dict_data;
    RtmResponseBase base;
    if (!RestfulDataParser::RtmParseResponseBase(root, base, dict_data, false) ||
        !dict_data.isValid()) {
      LOG_ERR_AND_RET_INT(
          ERR_FAILED, "failed to get JSON root in OnMessageReceivedFromPeer():\n----\n%s\n----\n",
          message.c_str());
    }

    auto parse_data = std::make_unique<RtmPeerMessage>();
    *static_cast<RtmResponseBase*>(parse_data.get()) = base;
    if (RestfulDataParser::RtmParsePeerMessage(dict_data, *parse_data)) {
      switch (parse_data->cmd) {
        case RTM_CMD_PEER_MESSAGE: {
          auto msg_data = std::move(*parse_data);
          agora_refptr<IAgoraMessage> send_msg = new RefCountedObject<AgoraMessage>();
          send_msg->SetMessage(msg_data.msg.c_str());
          send_msg->SetTimestamp(msg_data.ts);
          UserInfoWithOperator user_info;
          strncpy(user_info.user_info.user_name, msg_data.from_user.user_name.c_str(),
                  kMaxUserIdSize - 1);
          strncpy(user_info.user_info.user_id, msg_data.from_user.user_uuid.c_str(),
                  kMaxUserIdSize - 1);
          event_handlers_->Post(LOCATION_HERE, [user_info, send_msg](auto event_handler) {
            event_handler->OnMessageReceived(user_info.user_info.user_id, send_msg);
          });
        } break;

        default:
          LOG_INFO("OnMessageReceivedFromPeer unknown cmd: %d", parse_data->cmd);
          break;
      }
    }

    return ERR_OK_;
  });
}

}  // namespace rte
}  // namespace agora

//AGORA_API agora::rte::IAgoraRealTimeEngagement* AGORA_CALL createAgoraRealTimeEngagement() {
//  return new agora::rte::RteEngine;
//}
//
//AGORA_API const char* AGORA_CALL getRteVersion(int* build) {
//  if (!build) {
//    LOG_WARN("nullptr build, cannot get build number");
//  }
//
//  return getAgoraSdkVersion(build);
//}
