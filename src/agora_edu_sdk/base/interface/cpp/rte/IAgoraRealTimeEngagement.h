//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"
#include "IAgoraLog.h"
#include "IAgoraRteScene.h"

namespace agora {
namespace rte {

class IAgoraMessage;
class IAgoraMediaControl;
class IAgoraDiagnosticService;
class IAgoraLastmileProbeService;
class IAgoraRtmpStreamingService;
class IRteEventHandler;

enum ScenePreset {
  SCENE_PRESET_DEFAULT = 0,
  SCENE_PRESET_EDUCATION = 1,
  SCENE_PRESET_VIDEO_CONFERENCE = 2,
  SCENE_PRESET_ENTERTAINMENT = 3,
};

/**
 * The definition of EngineConfiguration.
 */
struct EngagementConfiguration {
  /**
   * The App ID issued to the developers by Agora. Apply for a new one from Agora if it is
   * missing from your kit.
   */
  AppId appid_or_token;
  /** User id. It will be used when join the scene. */
  UserId user_id;
  /** User name. It will be used when join the scene. */
  const char* user_name;
  /** Scene preset. */
  ScenePreset scene_preset;
  /**
   * - For Android, it is the context of Activity or Application.
   * - For Windows, it is the window handle of app. Once set, this parameter enables you to plug
   * or unplug the video devices while they are powered.
   */
  Context context;

  const char* log_file_path;
  commons::LOG_LEVEL log_level;
  unsigned int log_file_size;
  IRteEventHandler* event_handler;

  EngagementConfiguration()
      : appid_or_token(nullptr),
        user_id(nullptr),
        user_name(nullptr),
        scene_preset(SCENE_PRESET_DEFAULT),
        context(nullptr),
        log_file_path(nullptr),
        log_level(commons::LOG_LEVEL::LOG_LEVEL_INFO),
        log_file_size(5000),
        event_handler(nullptr) {}
};

struct SceneProperty {
  const char* key;
  const void* value;

  SceneProperty() : key(nullptr), value(nullptr) {}
};

struct SceneConfiguration {
  SceneId scene_uuid;

  SceneConfiguration() : scene_uuid(nullptr) {}
};

struct InvitationInfo {
  UserId sender_id;
  SceneId scene_id;

  InvitationInfo() : sender_id(nullptr), scene_id(nullptr) {}
};

class IInvitation : public RefCountInterface {
 public:
  virtual agora_refptr<IAgoraRteScene> AcceptAndJoin() = 0;
  virtual AgoraError Reject() = 0;

  virtual const InvitationInfo GetInfo() const = 0;

 protected:
  ~IInvitation() {}
};

class IRteEventHandler {
 public:
  virtual void OnInitializeSuccess() = 0;
  virtual void OnInitializeFailed() = 0;

  // message received before joining scene
  virtual void OnMessageReceived(UserId from_user, agora_refptr<IAgoraMessage> message) = 0;

  virtual void OnInvitationReceived(agora_refptr<IInvitation> invitation) = 0;

 protected:
  ~IRteEventHandler() {}
};

/**
 * The IAgoraRealTimeEngagement class.
 *
 * This class provides the main methods that can be invoked by your app.
 *
 * IRteEngine is the basic interface class of the Agora Native SDK.
 * Creating an IRteEngine object and then calling the methods of
 * this object enables you to use Agora Native SDK's communication functionality.
 */
class IAgoraRealTimeEngagement {
 public:
  /**
   * Initializes the Agora SDK service.
   *
   * In this method, you need to enter the issued Agora App ID to start initialization.
   * After creating an IRteEngine object, call this method to initialize the service
   * before calling any other methods.
   * @param config The Engine Configuration.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int Initialize(const EngagementConfiguration& config) = 0;

  /**
   * Releases the IAgoraRealTimeEngagement object.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual void Release() = 0;

  // Create agora RTE Scene, the scene will be created in OnSceneCreated() callback asynchronously.
  virtual agora_refptr<IAgoraRteScene> CreateAgoraRteScene(
      const SceneConfiguration& scene_config) = 0;

  // Agora RTE Media Control Objects
  virtual agora_refptr<IAgoraMediaControl> CreateAgoraMediaControl() = 0;

  // Agolet
  //virtual agora_refptr<IAgoraDiagnosticService> CreateDiagnosticService() = 0;
  //virtual agora_refptr<IAgoraLastmileProbeService> CreateLastmileProbeService() = 0;
  //virtual agora_refptr<IAgoraRtmpStreamingService> CreateRtmpStreamingService() = 0;

  virtual void RegisterEventHandler(IRteEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IRteEventHandler* event_handler) = 0;

 protected:
  virtual ~IAgoraRealTimeEngagement() {}
};

}  // namespace rte
}  // namespace agora

////////////////////////////////////////////////////////
/** \addtogroup createAgoraRealTimeEngagement
 @{
 */
////////////////////////////////////////////////////////

/** Creates the Real Time Enginement engine object and returns the pointer.

* @return Pointer of the RTE engine object.
*/
AGORA_API agora::rte::IAgoraRealTimeEngagement* AGORA_CALL createAgoraRealTimeEngagement();

/**
 * Gets the SDK version.
 * @param build The build number.
 * @return The version of the current SDK in the string format.
 */
AGORA_API const char* AGORA_CALL getRteVersion(int* build);

////////////////////////////////////////////////////////
/** @} */
////////////////////////////////////////////////////////
