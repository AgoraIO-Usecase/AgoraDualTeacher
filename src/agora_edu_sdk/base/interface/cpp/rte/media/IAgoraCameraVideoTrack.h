//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRteBase.h"

#include "IAgoraMediaTrack.h"

namespace agora {
namespace rte {
/**
 * The IAgoraCameraVideoTrack class, which provides access to a camera capturer.
 */
class IAgoraCameraVideoTrack : public IAgoraMediaTrack {
 public:
  enum CameraSource {
    CAMERA_FRONT,
    CAMERA_BACK,
  };

 public:
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
  /**
   * Sets the camera source.
   *
   * @note
   * This method applies to Android and iOS only.
   *
   * @param source The camera source that you want to capture: #CameraSource.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SetCameraSource(CameraSource source) = 0;

  /**
   * Gets the camera source.
   *
   * @note
   * This method applies to Android and iOS only.
   *
   * @return The camera source: #CameraSource.
   */
  virtual CameraSource GetCameraSource() = 0;
  /**
   * Switches the camera source, for example, from the front camera, to the rear camera.
   *
   * @note
   * This method applies to Android and iOS only.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual int SwitchCamera() = 0;
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)

  virtual int SetView(View view) = 0;

  virtual int SetRenderMode(media::base::RENDER_MODE_TYPE mode) = 0;

  virtual int SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) = 0;

 protected:
  ~IAgoraCameraVideoTrack() {}
};

}  // namespace rte
}  // namespace agora
