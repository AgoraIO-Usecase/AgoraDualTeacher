//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include "IAgoraMediaPlayerSource.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace rtc {

class IMediaPlayerSourceEx : public IMediaPlayerSource {
 protected:
  virtual ~IMediaPlayerSourceEx() = default;

 public:
  static agora_refptr<IMediaPlayerSource> Create(base::IAgoraService *agora_service,
                                                 utils::worker_type player_worker,
                                                 media::base::MEDIA_PLAYER_SOURCE_TYPE type = media::base::MEDIA_PLAYER_SOURCE_DEFAULT);

  virtual agora_refptr<rtc::IAudioPcmDataSender> getAudioPcmDataSender() = 0;
  virtual agora_refptr<rtc::IVideoFrameSender> getVideoFrameSender() = 0;
};

}  // namespace rtc
}  // namespace agora
