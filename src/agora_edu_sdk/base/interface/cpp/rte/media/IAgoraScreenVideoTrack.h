//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

#include "IAgoraMediaTrack.h"

namespace agora {
namespace rte {

class IAgoraScreenVideoTrack : public IAgoraMediaTrack {
 public:
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  /**
   * Initializes the screen capturer by specifying a display ID.
   *
   * @note
   * This method applies to macOS only.
   *
   * This method shares a whole or part of a screen specified by the display ID.
   * @param displayId The display ID of the screen to be shared. This parameter specifies which
   * screen you want to share.
   * @param regionRect The reference to the relative location of the region to the screen:
   * Rectangle.
   * - If the specified region overruns the screen, only the region within the screen will be
   * captured.
   * - If you set `width` or `height` as 0, the whole screen will be captured.
   * Note that the coordinate of rectangle is relative to the window and follow system specification
   * @return
   * - 0: Success.
   * - < 0: Failure.
   *   - ERR_INVALID_STATE, probably because there is already an active screen sharing, call
   * stopScreenCapture() first.
   *   - ERR_INVALID_ARGUMENT, if all the coordinates of `regionRect` are out of the specified
   * display.
   */
  virtual int InitWithDisplayId(View displayId, const rtc::Rectangle& regionRect) = 0;
#elif defined(_WIN32)
  /**
   * Initializes the screen capturer by specifying a screen Rect.
   *
   * @note
   * This method applies to Windows only.
   *
   * This method shares a whole or part of a screen specified by the screen Rect.
   * @param screenRect The reference to the Rect of the screen to be shared. This parameter
   * specifies which screen you want to share.
   * @param regionRect The reference to the relative location of the region to the screen:
   * regionRect.
   * - If the specified region overruns the screen, only the region within the screen will be
   * captured.
   * - If you set `width` or `height` as 0, the whole screen will be captured.
   * Note that the coordinate of rectangle is relative to the window and follow system specification
   * @return
   * - 0: Success.
   * - < 0: Failure.
   *   - ERR_INVALID_STATE, probably because there is already an active screen sharing, call
   * stopScreenCapture() first.
   *   - ERR_INVALID_ARGUMENT, if all the coordinates of `regionRect` are out of the specified
   * screen.
   */
  virtual int InitWithScreenRect(const rtc::Rectangle& screenRect,
                                 const rtc::Rectangle& regionRect) = 0;
#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE
  /**
   * Initializes the screen capturer by specifying a window ID.
   *
   * This method shares a whole or part of a window specified by the window ID.
   *
   * @note
   * This method applies to Windows and macOS only.
   * @param windowId The ID of the window to be shared. This parameter specifies which window you
   * want to share.
   * @param regionRect The reference to the relative location of the region to the window:
   * regionRect.
   * - If the specified region overruns the window, only the region within the screen will be
   * captured.
   * - If you set `width` or `height` as 0, the whole window will be captured.
   * Note that the coordinate of rectangle is relative to the window and follow system specification
   * @return
   * - 0: Success.
   * - < 0: Failure.
   *   - ERR_INVALID_STATE, probably because there is already an active screen sharing, call
   * stopScreenCapture() first.
   *   - ERR_INVALID_ARGUMENT if all the coordinates of `regionRect` are out of the specified
   * window.
   */
  virtual int InitWithWindowId(View windowId, const rtc::Rectangle& regionRect) = 0;

  /**
   * Sets the content hint for screen sharing.
   *
   * A content hint suggests the type of the content being shared, so that the SDK applies different
   * optimization algorithm to different types of content.
   * @param contentHint The content hint for screen capture: \ref rtc::VIDEO_CONTENT_HINT
   * "VIDEO_CONTENT_HINT".
   * @return
   * - 0: Success.
   * - < 0: Failure.
   *   - ERR_NOT_READY: No screen or window is being shared.
   */
  virtual int SetContentHint(rtc::VIDEO_CONTENT_HINT contentHint) = 0;

#if defined(_WIN32)
  /**
   * Updates the screen capture region.
   * @param regionRect The reference to the relative location of the region to the screen or window:
   * Rectangle.
   * - If the specified region overruns the screen or window, the screen capturer captures only the
   * region within it.
   * - If you set `width` or `height` as 0, the SDK shares the whole screen or window.
   * @return
   * - 0: Success.
   * - < 0: Failure.
   *   - No screen or window is being shared.
   */
  virtual int UpdateScreenCaptureRegion(const rtc::Rectangle& regionRect) = 0;
#endif  // _WIN32

  virtual int SetView(View view) = 0;

  virtual int SetRenderMode(media::base::RENDER_MODE_TYPE mode) = 0;

  virtual int SetVideoEncoderConfiguration(const rtc::VideoEncoderConfiguration& config) = 0;

 protected:
  ~IAgoraScreenVideoTrack() {}
};

}  // namespace rte
}  // namespace agora
