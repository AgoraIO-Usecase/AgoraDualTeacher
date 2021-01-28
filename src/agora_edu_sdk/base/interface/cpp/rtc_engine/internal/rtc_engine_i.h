//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <functional>
#include <string>
#include "base/AgoraBase.h"
#include "internal/IAgoraRtcEngine2.h"
#include "rtc/rtc_event.h"
#include "utils/thread/thread_pool.h"

#if defined(_WIN32)
extern HINSTANCE GetCurrentModuleInstance();
#elif defined(__ANDROID__) || defined(__linux__)
#include <dlfcn.h>
#endif

namespace agora {

namespace commons {
namespace network {
struct network_info_t;
}  // namespace network
}  // namespace commons

class VideoCapturerInterface;
class VideoRendererInterface;

namespace rtc {
struct VideoNetOptions;
namespace protocol {
struct CmdSeiLayout;
struct CmdInjectStreamConfig;
}  // namespace protocol

enum INTERFACE_ID_EX_TYPE {
  AGORA_IID_RTC_ENGINE_EX = 0xacbd,
};

enum SCREEN_SHARING_MODE { SCREEN_SHARING_NORMAL = 0, SCREEN_SHARING_MOVIE = 1 };

#ifdef INTERNAL_ENGINE_STATUS
struct InternalEngineStatus {
  int recfreq;
  int playoutfreq;
  int audio_send_frame_rate;
  int audio_send_packet_rate;
  int audio_recv_packet_rate;
  int nearin_signal_level;
  int nearout_signal_level;
  int farin_signal_level;
};
#endif  // INTERNAL_ENGINE_STATUS

enum class RecordingEventType : unsigned {
  RECORDING_EVENT_UNKNOWN,
  RECORDING_EVENT_START,
  RECORDING_EVENT_JOIN = RECORDING_EVENT_START,
  RECORDING_EVENT_LEAVE,

  RECORDING_EVENT_END = RECORDING_EVENT_LEAVE,
};

enum RecordingEventKeyIndex {
  // Join event
  RECORDING_JOIN_EVT_START = 1,
  RECORDING_JOIN_EVT_MIXMODE = RECORDING_JOIN_EVT_START,
  RECORDING_JOIN_EVT_MIXEDVIDEOAUDIOMODE = 2,
  RECORDING_JOIN_EVT_MIXHIGH = 3,
  RECORDING_JOIN_EVT_MIXLOW = 4,
  RECORDING_JOIN_EVT_MIXFPS = 5,
  RECORDING_JOIN_EVT_MIXKBPS = 6,
  RECORDING_JOIN_EVT_MINUDPPORT = 7,
  RECORDING_JOIN_EVT_MAXUDPPORT = 8,
  RECORDING_JOIN_EVT_DECODEAUDIOTYPE = 9,
  RECORDING_JOIN_EVT_DECODEVIDEOTYPE = 10,
  RECORDING_JOIN_EVT_LIVEMODE = 11,
  RECORDING_JOIN_EVT_IDLE = 12,
  RECORDING_JOIN_EVT_AUDIOONLY = 13,
  RECORDING_JOIN_EVT_VIDEOONLY = 14,  // NEW INSERT
  RECORDING_JOIN_EVT_SYSLOGFACILITY = 15,
  RECORDING_JOIN_EVT_STREAMTYPE = 16,
  RECORDING_JOIN_EVT_TRIGGERMODE = 17,
  RECORDING_JOIN_EVT_LANGUAGE = 18,
  RECORDING_JOIN_EVT_RESERVE1 = 19,

  // add new recording join event here ...

  MAX_RECORDING_JOIN_EVT_RESERVE15 = RECORDING_JOIN_EVT_RESERVE1 + 14,
  RECORDING_JOIN_EVT_MAX = MAX_RECORDING_JOIN_EVT_RESERVE15,

  // Leave event
  RECORDING_LEAVE_EVT_START = 101,
  RECORDING_LEAVE_EVT_LEAVEPATHCODE = RECORDING_LEAVE_EVT_START,
  RECORDING_LEAVE_EVT_RESERVE1,

  // add new recording leave event here ...

  RECORDING_LEAVE_EVT_RESERVE5 = RECORDING_LEAVE_EVT_RESERVE1 + 4,
  RECORDING_LEAVE_EVT_MAX = RECORDING_LEAVE_EVT_RESERVE5,
};

struct video_transport_packet_info {
    uid_t uid;
    unsigned int frame_num;
    unsigned int sent_ts;
    unsigned int ts;
};

struct audio_transport_packet_info {
    uid_t uid;
    unsigned int ts;
    unsigned int sent_ts;
};

class IRtcAvTransportPacketInfoObserver {
public:
  virtual ~IRtcAvTransportPacketInfoObserver() = default;

  virtual void onVideoTransportPacketInfo(const video_transport_packet_info& info) = 0;
  virtual void onAudioTransportPacketInfo(const audio_transport_packet_info& info) = 0;
};

class IRtcEngineEventHandlerEx : public IRtcEngineEventHandler2 {
 public:
  virtual void onMediaEngineLoadSuccess() {}
  virtual void onMediaEngineStartCallSuccess() {}
  virtual void onAudioTransportQuality(user_id_t userId, unsigned int bitrate, unsigned short delay,
                                       unsigned short lost) {
    (void)userId;
    (void)bitrate;
    (void)delay;
    (void)lost;
  }

  virtual void onVideoTransportQuality(user_id_t userId, unsigned int bitrate, unsigned short delay,
                                       unsigned short lost) {
    (void)userId;
    (void)bitrate;
    (void)delay;
    (void)lost;
  }

  virtual void onRecap(const char* recapData, int length) {
    (void)recapData;
    (void)length;
  }

  virtual bool onEvent(RTC_EVENT evt, std::string* payload) {
    (void)evt;
    (void)payload;

    /* return false to indicate this event is not handled */
    return false;
  }

  virtual void onMediaEngineEvent(int evt) { (void)evt; }

  virtual bool onCustomizedSei(const void** content, int* length) {
    (void)content;
    (void)length;

    /* return false to indicate the SEI content is left to SDK to generate */
    return false;
  }

#ifdef INTERNAL_ENGINE_STATUS
  virtual void onInternalEngineStatus(InternalEngineStatus state) { (void)state; }
#endif  // INTERNAL_ENGINE_STATUS
};

struct RtcEngineContextEx : public RtcEngineContext {
  bool isExHandler;
  bool useStringUid;
  bool forceAlternativeNetworkEngine;
  conn_id_t connectionId;
  int maxOutputBitrateKpbs;

  // 0 for AGORA_CC,
  // 1 for REMB
  // 2 for TRANSPORT_CC
  int ccType;
  bool is_p2p_switch_enabled_;

  RtcEngineContextEx()
      : RtcEngineContext(),
        isExHandler(false),
        useStringUid(true),
#if defined(_WIN32)
        forceAlternativeNetworkEngine(true),
#else
        forceAlternativeNetworkEngine(false),
#endif
        connectionId(0),
        maxOutputBitrateKpbs(30 * 1000),
        ccType(0),
#if defined(P2P_SWITCH_DEFAULT_VALUE)
        is_p2p_switch_enabled_(P2P_SWITCH_DEFAULT_VALUE)
#else
        is_p2p_switch_enabled_(false)
#endif
        {}

  explicit RtcEngineContextEx(const RtcEngineContext& ctx)
      : RtcEngineContext(ctx),
        isExHandler(false),
        useStringUid(false),
#if defined(_WIN32)
        forceAlternativeNetworkEngine(true),
#else
        forceAlternativeNetworkEngine(false),
#endif
        connectionId(0),
        maxOutputBitrateKpbs(30 * 1000),
        ccType(0),
#if defined(P2P_SWITCH_DEFAULT_VALUE)
        is_p2p_switch_enabled_(P2P_SWITCH_DEFAULT_VALUE)
#else
        is_p2p_switch_enabled_(false)
#endif
      {}
};

struct WebAgentVideoStats {
  user_id_t userId;
  int delay;
  int sentFrameRate;
  int renderedFrameRate;
  int skippedFrames;
};
class RtcContext;

// full feature definition of rtc engine interface
class IRtcEngineEx : public IRtcEngine2,
                     public agora::base::IParameterEngine {
 public:
  virtual ~IRtcEngineEx() {}
  virtual int initializeEx(const RtcEngineContextEx& context) = 0;

  virtual int muteRemoteAudioStream(uid_t userId, bool mute,
                                    conn_id_t connectionId = DEFAULT_CONNECTION_ID) = 0;

  virtual int muteRemoteVideoStream(uid_t userId, bool mute,
                                    conn_id_t connectionId = DEFAULT_CONNECTION_ID) = 0;

  virtual int addVideoWatermark(const RtcImage& watermark) = 0;
  virtual int clearVideoWatermarks() = 0;
  virtual int setProfile(const char* profile, bool merge) = 0;
  virtual int getProfile(any_document_t& result) = 0;
  virtual int notifyNetworkChange(agora::commons::network::network_info_t&& networkInfo) = 0;
  virtual int getOptionsByVideoProfile(int profile, VideoNetOptions& options) = 0;

  virtual int setAudioOptionParams(const char* params) = 0;
  virtual int getAudioOptionParams(char* params) = 0;
  virtual int setAudioSessionParams(const char* params) = 0;
  virtual int getAudioSessionParams(char* params) = 0;
  virtual bool isMicrophoneOn() = 0;
  /**
   * get SHA1 values of source files for building the binaries being used, for
   * bug tracking.
   */
  // virtual const char* getSourceVersion() const = 0;

  virtual int reportWebAgentVideoStats(const WebAgentVideoStats& stats) = 0;

#if (defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IOS)
  virtual int monitorDeviceChange(bool enabled) = 0;
#endif

  virtual int printLog(int level, const char* message) = 0;
  virtual int runOnWorkerThread(std::function<void(void)>&& f) = 0;
  virtual int reportArgusCounters(int* counterId, int* value, int count, user_id_t userId) = 0;

  virtual int reportRecordingArgusEvent(uint32_t* eventIds, int64_t* value, int count,
                                        RecordingEventType eventType) = 0;
  virtual int addInjectStreamUrl2(const char* url, protocol::CmdInjectStreamConfig& config) = 0;
  virtual int enableYuvDumper(bool enable) = 0;

  // this function is only for expert of video codec,
  // please do NOT call it if not knowing what's it about
  virtual int setVideoConfigParam(const char* params) = 0;

  virtual int getVideoConfigParam(char* params, size_t size) = 0;

  virtual int setExternalVideoConfigEx(const VideoEncoderConfiguration& config, conn_id_t connectionId) = 0;

  // this function is only for switching screen capture source api on windows platform
#if defined(_WIN32)
  virtual void SetScreenCaptureSource(bool allow_magnification_api, bool allow_directx_capturer) = 0;
#endif

  virtual int setLogLevelEx(unsigned int filter) = 0;

  virtual int simulateOnSetParameters(const std::string& parameters) = 0;

  virtual int setCameraDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) = 0;
};

class RtcEngineLibHelper {
  typedef const char*(AGORA_CALL* PFN_GetAgoraRtcEngineVersion)(int* build);
  typedef IRtcEngine*(AGORA_CALL* PFN_CreateAgoraRtcEngine)();
  typedef void(AGORA_CALL* PFN_ShutdownAgoraRtcEngineService)();
#if defined(_WIN32)
  typedef HINSTANCE lib_handle_t;
  static HINSTANCE MyLoadLibrary(const char* dllname) {
    char path[MAX_PATH];
    GetModuleFileNameA(GetCurrentModuleInstance(), path, MAX_PATH);
    auto p = strrchr(path, '\\');
    strcpy(p + 1, dllname);  // NOLINT
    HINSTANCE hDll = LoadLibraryA(path);
    if (hDll) return hDll;

    hDll = LoadLibraryA(dllname);
    return hDll;
  }
#else
  typedef void* lib_handle_t;
#endif

 public:
  RtcEngineLibHelper(const char* dllname)
      : m_firstInit(true),
        m_lib(NULL),
        m_dllName(dllname),
        m_pfnCreateAgoraRtcEngine(nullptr),
        m_pfnGetAgoraRtcEngineVersion(nullptr) {}

  bool initialize() {
    if (!m_firstInit) return isValid();
    m_firstInit = false;
#if defined(_WIN32)
    m_lib = MyLoadLibrary(m_dllName.c_str());
    if (m_lib) {
      m_pfnCreateAgoraRtcEngine =
          (PFN_CreateAgoraRtcEngine)GetProcAddress(m_lib, "createAgoraRtcEngine");
      m_pfnGetAgoraRtcEngineVersion =
          (PFN_GetAgoraRtcEngineVersion)GetProcAddress(m_lib, "getAgoraRtcEngineVersion");
    }
#elif defined(__ANDROID__) || defined(__linux__)
    m_lib = dlopen(m_dllName.c_str(), RTLD_LAZY);
    if (m_lib) {
      m_pfnCreateAgoraRtcEngine = (PFN_CreateAgoraRtcEngine)dlsym(m_lib, "createAgoraRtcEngine");
      m_pfnGetAgoraRtcEngineVersion =
          (PFN_GetAgoraRtcEngineVersion)dlsym(m_lib, "getAgoraRtcEngineVersion");
    }
#endif
    return isValid();
  }

  void deinitialize() {
    if (m_lib) {
#if defined(_WIN32)
      FreeLibrary(m_lib);
#elif defined(__ANDROID__) || defined(__linux__)
      dlclose(m_lib);
#endif
      m_lib = NULL;
    }
    m_pfnCreateAgoraRtcEngine = nullptr;
    m_pfnGetAgoraRtcEngineVersion = nullptr;
  }

  ~RtcEngineLibHelper() { deinitialize(); }

  bool isValid() { return m_pfnCreateAgoraRtcEngine != NULL; }

  agora::rtc::IRtcEngine* createEngine() {
    return m_pfnCreateAgoraRtcEngine ? m_pfnCreateAgoraRtcEngine() : NULL;
  }
  const char* getVersion(int* build) {
    return m_pfnGetAgoraRtcEngineVersion ? m_pfnGetAgoraRtcEngineVersion(build) : nullptr;
  }

 private:
  bool m_firstInit;
  lib_handle_t m_lib;
  std::string m_dllName;
  PFN_CreateAgoraRtcEngine m_pfnCreateAgoraRtcEngine;
  PFN_GetAgoraRtcEngineVersion m_pfnGetAgoraRtcEngineVersion;
};

// A helper function for decoding out the SEI layout

struct canvas_info {
  int width;
  int height;
  int bgcolor;
};

struct region_info {
  unsigned id;

  double x;
  double y;
  double width;
  double height;

  int alpha;  // [0, 255]

  int render_mode;  // 0, crop; 1, ZoomtoFit
  int zorder;       // [0, 100]
};

#define NUM_OF_LAYOUT_REGIONS 17
struct layout_info {
  long long ms;
  canvas_info canvas;
  // At most 9 broadcasters: 1 host, 8 guests.
  unsigned int region_count;
  region_info regions[NUM_OF_LAYOUT_REGIONS];
};

AGORA_API bool AGORA_CALL decode_sei_layout(const void* data, unsigned size, layout_info* layout);
}  // namespace rtc
}  // namespace agora
