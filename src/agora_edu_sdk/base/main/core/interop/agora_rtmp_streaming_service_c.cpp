//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "api2/IAgoraRtmpStreamingService.h"
#include "api2/agora_rtmp_streaming_service.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/agora_base.h"

namespace {

void copy_transcoding_users_from_c(agora::rtc::TranscodingUser* cpp_user,
                                   const transcoding_user& user) {
  if (!cpp_user) {
    return;
  }

  cpp_user->alpha = user.alpha;
  cpp_user->audioChannel = user.audio_channel;
  cpp_user->height = user.height;
  cpp_user->uid = user.uid;
  cpp_user->width = user.width;
  cpp_user->x = user.x;
  cpp_user->y = user.y;
  cpp_user->zOrder = user.z_order;
}

void copy_rtc_image_from_c(agora::rtc::RtcImage* cpp_image, const rtc_image& image) {
  if (!cpp_image) {
    return;
  }

  cpp_image->x = image.x;
  cpp_image->y = image.y;
  cpp_image->height = image.height;
  cpp_image->width = image.width;
  cpp_image->url = image.url;
}

void copy_live_transcoding_from_c(agora::rtc::LiveTranscoding* cpp_livetrans,
                                  const live_transcoding& livetrans) {
  if (!cpp_livetrans) {
    return;
  }

  cpp_livetrans->width = livetrans.width;
  cpp_livetrans->height = livetrans.height;
  cpp_livetrans->lowLatency = livetrans.low_latency;
  cpp_livetrans->metadata = livetrans.metadata;
  cpp_livetrans->transcodingExtraInfo = livetrans.transcoding_extra_info;

  copy_transcoding_users_from_c(cpp_livetrans->transcodingUsers, *(livetrans.transcoding_users));

  cpp_livetrans->userCount = livetrans.user_count;
  cpp_livetrans->videoBitrate = livetrans.video_bitrate;

  cpp_livetrans->videoCodecProfile =
      static_cast<agora::rtc::VIDEO_CODEC_PROFILE_TYPE>(livetrans.video_codec_profile);

  cpp_livetrans->videoFramerate = livetrans.video_frame_rate;
  cpp_livetrans->videoGop = livetrans.video_gop;

  copy_rtc_image_from_c(cpp_livetrans->watermark, *(livetrans.watermark));

  cpp_livetrans->audioBitrate = livetrans.audio_bitrate;
  cpp_livetrans->audioChannels = livetrans.audio_channels;
  cpp_livetrans->audioCodecProfile =
      static_cast<agora::rtc::AUDIO_CODEC_PROFILE_TYPE>(livetrans.audio_codec_profile);
  cpp_livetrans->audioSampleRate =
      static_cast<agora::rtc::AUDIO_SAMPLE_RATE_TYPE>(livetrans.audio_sample_rate);
  cpp_livetrans->backgroundColor = livetrans.background_color;

  copy_rtc_image_from_c(cpp_livetrans->backgroundImage, *(livetrans.background_image));
}

class CRtmpStreamingObserver : public agora::rtc::IRtmpStreamingObserver,
                               public agora::interop::CAgoraCallback<rtmp_streaming_observer> {
 public:
  CRtmpStreamingObserver() = default;
  ~CRtmpStreamingObserver() override = default;

  void onRtmpStreamingStateChanged(const char* url, agora::rtc::RTMP_STREAM_PUBLISH_STATE state,
                                   agora::rtc::RTMP_STREAM_PUBLISH_ERROR errCode) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_rtmp_streaming_state_changed) {
        p.second.on_rtmp_streaming_state_changed(p.first, url, state, errCode);
      }
    }
  }

  void onStreamPublished(const char* url, int error) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_stream_published) {
        p.second.on_stream_published(p.first, url, error);
      }
    }
  }

  void onStreamUnpublished(const char* url) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_stream_unpublished) {
        p.second.on_stream_unpublished(p.first, url);
      }
    }
  }

  void onTranscodingUpdated() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_transcoding_updated) {
        p.second.on_transcoding_updated(p.first);
      }
    }
  }
};

CRtmpStreamingObserver g_rtmp_streaming_central_observer;

}  // namespace

AGORA_API_C_INT agora_rtmp_streaming_service_add_publish_stream_url(
    AGORA_HANDLE agora_rtmp_streaming_service, const char* url, int transcoding_enabled) {
  if (!agora_rtmp_streaming_service) {
    return -1;
  }

  REF_PTR_HOLDER_CAST(rtmp_streaming_service_holder, agora::rtc::IRtmpStreamingService,
                      agora_rtmp_streaming_service);
  return rtmp_streaming_service_holder->Get()->addPublishStreamUrl(url, transcoding_enabled);
}

AGORA_API_C_INT agora_rtmp_streaming_service_remove_publish_stream_url(
    AGORA_HANDLE agora_rtmp_streaming_service, const char* url) {
  if (!agora_rtmp_streaming_service) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(rtmp_streaming_service_holder, agora::rtc::IRtmpStreamingService,
                      agora_rtmp_streaming_service);

  return rtmp_streaming_service_holder->Get()->removePublishStreamUrl(url);
}

AGORA_API_C_INT agora_rtmp_streaming_service_set_live_transcoding(
    AGORA_HANDLE agora_rtmp_streaming_service, const live_transcoding* transcoding) {
  if (!agora_rtmp_streaming_service || !transcoding) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(rtmp_streaming_service_holder, agora::rtc::IRtmpStreamingService,
                      agora_rtmp_streaming_service);

  agora::rtc::LiveTranscoding cpp_trans;

  copy_live_transcoding_from_c(&cpp_trans, *transcoding);

  return rtmp_streaming_service_holder->Get()->setLiveTranscoding(cpp_trans);
}

AGORA_API_C_INT agora_rtmp_streaming_service_register_observer(
    AGORA_HANDLE agora_rtmp_streaming_service, rtmp_streaming_observer* observer) {
  if (!agora_rtmp_streaming_service || !observer) {
    return -1;
  }
  g_rtmp_streaming_central_observer.Add(agora_rtmp_streaming_service, observer);
  return 0;
}

AGORA_API_C_INT agora_rtmp_streaming_service_unregister_observer(
    AGORA_HANDLE agora_rtmp_streaming_service, rtmp_streaming_observer* observer) {
  if (!agora_rtmp_streaming_service || !observer) {
    return -1;
  }
  g_rtmp_streaming_central_observer.Remove(agora_rtmp_streaming_service);
  return 0;
}
