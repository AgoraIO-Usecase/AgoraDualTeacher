//
//  EduVideoEncoderConfiguration.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

enum EduVideoOutputOrientationMode {
  /** Adaptive mode (Default).
   <p>The video encoder adapts to the orientation mode of the video input
   device. When you use a custom video source, the output video from the encoder
   inherits the orientation of the original video. <ul><li>If the width of the
   captured video from the SDK is greater than the height, the encoder sends the
   video in landscape mode. The encoder also sends the rotational information of
   the video, and the receiver uses the rotational information to rotate the
   received video.</li> <li>If the original video is in portrait mode, the
   output video from the encoder is also in portrait mode. The encoder also
   sends the rotational information of the video to the receiver.</li></ul></p>
   */
  EDU_VIDEO_OUTPUT_ORIENTATION_MODE_ADAPTATIVE,
  /** Landscape mode.
   <p>The video encoder always sends the video in landscape mode. The video
   encoder rotates the original video before sending it and the rotational
   information is 0. This mode applies to scenarios involving CDN live
   streaming.</p>
   */
  EDU_VIDEO_OUTPUT_ORIENTATION_MODE_FIXED_LANDSCAPE,
  /** Portrait mode.
   <p>The video encoder always sends the video in portrait mode. The video
   encoder rotates the original video before sending it and the rotational
   information is 0. This mode applies to scenarios involving CDN live
   streaming.</p>
   */
  EDU_VIDEO_OUTPUT_ORIENTATION_MODE_FIXED_PORTRAIT
};

/** The video encoding degradation preference under limited bandwidth. */
enum EduDegradationPreference {
  /** (Default) Degrades the frame rate to guarantee the video quality. */
  EDU_DEGRADATION_MAINTAIN_QUALITY,
  /** Degrades the video quality to guarantee the frame rate. */
  EDU_DEGRADATION_MAINTAIN_FRAMERATE,
  /** Reserved for future use. */
  EDU_DEGRADATION_BALANCED
};