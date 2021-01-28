//
//  Agora SDK
//
//  Created by Sting Feng in 2017-11.
//  Copyright (c) 2017 Agora.io. All rights reserved.
//
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "api2/NGIAgoraExtensionControl.h"
#include "api2/NGIAgoraVideoTrack.h"
#include "api2/internal/agora_service_i.h"
#include "api2/internal/media_node_factory_i.h"
#include "base/base_context.h"
#include "extension_control_impl.h"
#include "facilities/miscellaneous/system_error_handler.h"
#include "internal/rtc_engine_i.h"

namespace cricket {
struct AudioOptions;
}  // namespace cricket

namespace agora {
namespace commons {
class TraceCallbackBridge;
}  // namespace commons

namespace rtc {
class ConfigService;
class AgoraGenericBridge;
}  // namespace rtc

namespace utils {
class IRtcStatsReporter;
class IRtcEventReporter;
}  // namespace utils

namespace base {
class BaseContext;
class UserAccountClient;

class AgoraService : public IAgoraServiceEx {
 public:
  static AgoraService* Create();

  static AgoraService* Get();

  int initialize(const AgoraServiceConfiguration& context) override;
  int release() override;
  int setLogFile(const char* filePath, unsigned int fileSize) override;
  int setLogFilter(unsigned int filters) override;
  rtm::IRtmService* createRtmService() override;

 public:  // IAgoraServiceEx
  int initializeEx(const AgoraServiceConfigEx& configEx) override;
  agora_refptr<rtc::IRtcConnection> createRtcConnectionEx(
      const rtc::RtcConnectionConfigurationEx& cfg) override;
  int panic(void* exception) override;
  event_base* getWorkerEventBase() override;

  rtc::AgoraGenericBridge* getBridge() override {
    if (context_) {
      return context_->getBridge();
    }
    return nullptr;
  }

  BaseContext& getBaseContext() override { return *context_; }
  // make it testable
  void setBaseContext(BaseContext* context) override {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
    context_.reset(context);
#endif
  }
  int32_t setLogWriter(commons::ILogWriter* logWriter) override;
  commons::ILogWriter* releaseLogWriter() override;
  rtc::ConfigService* getConfigService() override;
  agora_refptr<rtc::PredefineIpList> getPredefineIpList() const override;

  agora_refptr<rtc::IRtcConnection> getOneRtcConnection(bool admBinded) const override;

  const std::string& getAppId() const override;

  bool useStringUid() const override { return use_string_uid_; }
  void registerLocalUserAccount(const char* app_id, const char* user_account) override;
  rtc::uid_t getUidByUserAccount(const std::string& user_account) const override;

 public:
  int setAudioSessionPreset(rtc::AUDIO_SCENARIO_TYPE scenario) override;
  int setAudioSessionConfiguration(const AudioSessionConfiguration& config) override;
  int getAudioSessionConfiguration(AudioSessionConfiguration* config) override;

  agora_refptr<rtc::IRtcConnection> createRtcConnection(
      const rtc::RtcConnectionConfiguration& cfg) override;

  agora_refptr<rtc::ILocalAudioTrack> createLocalAudioTrack() override;

  agora_refptr<rtc::ILocalAudioTrack> createCustomAudioTrack(
      agora_refptr<rtc::IAudioPcmDataSender> audioSource) override;

  agora_refptr<rtc::ILocalAudioTrack> createCustomAudioTrack(
      agora_refptr<rtc::IAudioEncodedFrameSender> audioSource, TMixMode mixMode) override;

  agora_refptr<rtc::ILocalAudioTrack> createCustomAudioTrack(
      agora_refptr<rtc::IMediaPacketSender> source) override;

  agora_refptr<rtc::ILocalAudioTrack> createMediaPlayerAudioTrack(
      agora_refptr<rtc::IMediaPlayerSource> audioSource) override;

  agora_refptr<rtc::ILocalAudioTrack> createRecordingDeviceAudioTrack(
      agora_refptr<rtc::IRecordingDeviceSource> audioSource) override;

  agora_refptr<rtc::INGAudioDeviceManager> createAudioDeviceManager() override;

  agora_refptr<rtc::IMediaNodeFactory> createMediaNodeFactory() override;

  agora_refptr<rtc::ILocalVideoTrack> createCameraVideoTrack(
      agora_refptr<rtc::ICameraCapturer> videoSource) override;

  agora_refptr<rtc::ILocalVideoTrack> createScreenVideoTrack(
      agora_refptr<rtc::IScreenCapturer> videoSource) override;

  agora_refptr<rtc::ILocalVideoTrack> createMixedVideoTrack(
      agora_refptr<rtc::IVideoMixerSource> videoSource) override;

  agora_refptr<rtc::ILocalVideoTrack> createTranscodedVideoTrack(
      agora_refptr<rtc::IVideoFrameTransceiver> transceiver) override;

  agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(
      agora_refptr<rtc::IVideoFrameSender> videoSource) override;

  agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(
      agora_refptr<rtc::IVideoEncodedImageSender> videoSource, SenderOptions& options) override;

  agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(
      agora_refptr<rtc::IMediaPacketSender> source) override;

  agora_refptr<rtc::ILocalVideoTrack> createMediaPlayerVideoTrack(
      agora_refptr<rtc::IMediaPlayerSource> videoSource) override;

  agora_refptr<rtc::IRtmpStreamingService> createRtmpStreamingService(
      agora_refptr<rtc::IRtcConnection> rtcConnection, const char* appId) override;

  rtc::IExtensionControl* getExtensionControl() override;

  rtc::IDiagnosticService* getDiagnosticService() const override;

 public:
  static bool isValidChannelId(const std::string& name);
  static void printVersionInfo();

  // Should only be used by RtcConnectionImpl
  int unregisterRtcConnection(rtc::conn_id_t id);

 private:
  AgoraService();
  virtual ~AgoraService();
  void startLogService(const char* configLogDir);

  void updateConnectionConfig(rtc::RtcConnectionConfiguration& cfg);
  int registerRtcConnection(rtc::conn_id_t id, rtc::IRtcConnection* connection);
#if defined(WEBRTC_WIN)
  void overrideScreenCaptureSourceByCdsConfig(agora_refptr<rtc::IScreenCapturer> videoSource);
#endif

  int build_number_ = 0;
  const char* version_number_ = nullptr;
  std::unique_ptr<BaseContext> context_;
  std::atomic<rtc::conn_id_t> connId_{0};
  std::unique_ptr<commons::TraceCallbackBridge> traceCallbackBridge_;
  std::atomic<bool> initialized_ = {false};
  // NOTE: Read the following if you want to add objects in agora service.
  // Agora service should be a factory, which maintains no objects, put global base objects in
  // BaseContext instead.
  agora_refptr<rtc::IMediaNodeFactoryEx> media_node_factory_ex_;
  std::unique_ptr<rtc::ConfigService> config_service_;
  using ExtensionControlPtr =
      std::unique_ptr<rtc::ExtensionControlImpl, void (*)(rtc::ExtensionControlImpl*)>;
  ExtensionControlPtr extension_control_;
  std::unordered_map<rtc::conn_id_t, rtc::IRtcConnection*> connections_;
  agora_refptr<rtc::PredefineIpList> predefine_ip_list_;
  std::shared_ptr<utils::SystemErrorHandler> system_error_handler_;
  std::unique_ptr<utils::IRtcStatsReporter> generic_stats_reporter_;
  std::unique_ptr<utils::IRtcEventReporter> generic_event_reporter_;
  bool use_string_uid_;
  std::unique_ptr<base::UserAccountClient> user_account_client_;
};

}  // namespace base
}  // namespace agora
