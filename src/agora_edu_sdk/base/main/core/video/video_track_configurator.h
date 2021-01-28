//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once
#include <memory>
#include <string>
#include "base/AgoraBase.h"

namespace agora {
namespace rtc {

enum class ContentHint { NONE, MOTION, DETAIL };

struct VideoTrackConfigValue {
 public:
  enum class ConfigType { NONE, SCREEN_REGION, SCREEN_CAP_PARAMS, CONTENT_HINT };

  union ConfigValue {
    ContentHint content_hint_;
    Rectangle screen_rect_;
    ScreenCaptureParameters screen_cap_params_;
  };

  ConfigType config_type_ = ConfigType::NONE;
  ConfigValue config_value_ = {ContentHint::NONE};

  VideoTrackConfigValue() = default;
  ~VideoTrackConfigValue() = default;
  explicit VideoTrackConfigValue(ContentHint content_hint)
      : config_type_{ConfigType::CONTENT_HINT} {
    config_value_.content_hint_ = content_hint;
  }

  explicit VideoTrackConfigValue(const Rectangle& rect) : config_type_{ConfigType::SCREEN_REGION} {
    config_value_.screen_rect_ = rect;
  }

  explicit VideoTrackConfigValue(const ScreenCaptureParameters& screen_cap_params)
      : config_type_{ConfigType::SCREEN_CAP_PARAMS} {
    config_value_.screen_cap_params_ = screen_cap_params;
  }
};

class VideoTrackConfigurator {
 public:
  class IVideoTrackConfiguratorListener {
   public:
    virtual bool OnConfigChange(const VideoTrackConfigValue& config) = 0;
  };
  explicit VideoTrackConfigurator(IVideoTrackConfiguratorListener* listener)
      : listener_(listener) {}
  ~VideoTrackConfigurator() = default;

  template <class... TValue>
  bool UpdateConfig(TValue&&... config) {
    if (listener_) {
      return listener_->OnConfigChange(VideoTrackConfigValue{std::forward<TValue>(config)...});
    }
    return false;
  }

 private:
  IVideoTrackConfiguratorListener* listener_ = nullptr;
};

}  // namespace rtc
}  // namespace agora
