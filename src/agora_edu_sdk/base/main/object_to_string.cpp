//
//  Agora Media SDK
//
//  Created by minbo in 2020-06.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "object_to_string.h"
#include <sstream>

namespace agora {
namespace rtc {

std::string ObjectToString::channelMediaOptionsToString(const ChannelMediaOptions& options) {
  std::stringstream ss;
  ss << "publishAudio:" << optionalToString(options.publishAudioTrack);
  ss << " publishCustomAudio:" << optionalToString(options.publishCustomAudioTrack);
  ss << " publishMediaPlayerAudio:" << optionalToString(options.publishMediaPlayerAudioTrack);
  ss << " publishCamera:" << optionalToString(options.publishCameraTrack);
  ss << " publishScreen:" << optionalToString(options.publishScreenTrack);
  ss << " publishCustomVideo:" << optionalToString(options.publishCustomVideoTrack);
  ss << " publishEncodedVideo:" << optionalToString(options.publishEncodedVideoTrack);
  ss << " publishMediaPlayerVideo:" << optionalToString(options.publishMediaPlayerVideoTrack);
  ss << " publishMediaPlayerId:" << optionalToString(options.publishMediaPlayerId);
  ss << " autoSubscribeAudio:" << optionalToString(options.autoSubscribeAudio);
  ss << " autoSubscribeVideo:" << optionalToString(options.autoSubscribeVideo);
  ss << " enableAudioRecordingOrPlayout:"
     << optionalToString(options.enableAudioRecordingOrPlayout);
  ss << " clientRoleType:" << optionalToString(options.clientRoleType);
  ss << " defaultVideoStreamType:" << optionalToString(options.defaultVideoStreamType);
  ss << " channelProfile:" << optionalToString(options.channelProfile);
  return ss.str();
}

}  // namespace rtc
}  // namespace agora
