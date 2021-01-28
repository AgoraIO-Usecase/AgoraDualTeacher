//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "video_profile.h"
#include <rtc/media_engine.h>
#include <rtc/rtc_context.h>

namespace agora {
namespace rtc {
struct video_profile_t {
  int profile;
  int16_t width;
  int16_t height;
  int16_t frameRate;
  int16_t bitRate;
};

static video_profile_t s_videoProfiles[] = {
    {VIDEO_PROFILE_EX_SLOT, 0, 0, 0, 0},
    {VIDEO_PROFILE_ENGINE_SET_SLOT, 0, 0, 0, 0},
    {VIDEO_PROFILE_LANDSCAPE_120P, 160, 120, 15, 65},
    {VIDEO_PROFILE_LANDSCAPE_120P_3, 120, 120, 15, 50},
    //    { VIDEO_PROFILE_LANDSCAPE_96P, 128, 96, 10, 40 },
    //    { VIDEO_PROFILE_LANDSCAPE_48P, 64, 48, 10, 20 },
    {VIDEO_PROFILE_LANDSCAPE_180P, 320, 180, 15, 140},
    {VIDEO_PROFILE_LANDSCAPE_180P_3, 180, 180, 15, 100},
    {VIDEO_PROFILE_LANDSCAPE_180P_4, 240, 180, 15, 120},
    {VIDEO_PROFILE_LANDSCAPE_240P, 320, 240, 15, 200},
    {VIDEO_PROFILE_LANDSCAPE_240P_3, 240, 240, 15, 140},
    {VIDEO_PROFILE_LANDSCAPE_240P_4, 424, 240, 15, 220},
    {VIDEO_PROFILE_LANDSCAPE_360P, 640, 360, 15, 400},
    {VIDEO_PROFILE_LANDSCAPE_360P_3, 360, 360, 15, 260},
    {VIDEO_PROFILE_LANDSCAPE_360P_4, 640, 360, 30, 600},
    {VIDEO_PROFILE_LANDSCAPE_360P_6, 360, 360, 30, 400},
    {VIDEO_PROFILE_LANDSCAPE_360P_7, 480, 360, 15, 320},
    {VIDEO_PROFILE_LANDSCAPE_360P_8, 480, 360, 30, 490},
    {VIDEO_PROFILE_LANDSCAPE_360P_9, 640, 360, 15, 800},
    {VIDEO_PROFILE_LANDSCAPE_360P_10, 640, 360, 24, 800},
    {VIDEO_PROFILE_LANDSCAPE_360P_11, 640, 360, 24, 1000},
    {VIDEO_PROFILE_LANDSCAPE_480P, 640, 480, 15, 500},
    {VIDEO_PROFILE_LANDSCAPE_480P_3, 480, 480, 15, 400},
    {VIDEO_PROFILE_LANDSCAPE_480P_4, 640, 480, 30, 750},
    {VIDEO_PROFILE_LANDSCAPE_480P_6, 480, 480, 30, 600},
    {VIDEO_PROFILE_LANDSCAPE_480P_8, 848, 480, 15, 610},
    {VIDEO_PROFILE_LANDSCAPE_480P_9, 848, 480, 30, 930},
    {VIDEO_PROFILE_LANDSCAPE_480P_10, 640, 480, 10, 400},
    {VIDEO_PROFILE_LANDSCAPE_720P, 1280, 720, 15, 1130},
    {VIDEO_PROFILE_LANDSCAPE_720P_3, 1280, 720, 30, 1710},
    {VIDEO_PROFILE_LANDSCAPE_720P_5, 960, 720, 15, 910},
    {VIDEO_PROFILE_LANDSCAPE_720P_6, 960, 720, 30, 1380},
    {VIDEO_PROFILE_LANDSCAPE_1080P, 1920, 1080, 15, 2080},
    {VIDEO_PROFILE_LANDSCAPE_1080P_3, 1920, 1080, 30, 3150},
    {VIDEO_PROFILE_LANDSCAPE_1080P_5, 1920, 1080, 60, 4780},
    {VIDEO_PROFILE_LANDSCAPE_1440P, 2560, 1440, 30, 4850},
    {VIDEO_PROFILE_LANDSCAPE_1440P_2, 2560, 1440, 60, 7350},
    {VIDEO_PROFILE_LANDSCAPE_4K, 3840, 2160, 30, 8190},
    {VIDEO_PROFILE_LANDSCAPE_4K_3, 3840, 2160, 60, 13500},

    {VIDEO_PROFILE_PORTRAIT_120P, 120, 160, 15, 65},
    {VIDEO_PROFILE_PORTRAIT_120P_3, 120, 120, 15, 50},
    {VIDEO_PROFILE_PORTRAIT_180P, 180, 320, 15, 140},
    {VIDEO_PROFILE_PORTRAIT_180P_3, 180, 180, 15, 100},
    {VIDEO_PROFILE_PORTRAIT_180P_4, 180, 240, 15, 120},
    {VIDEO_PROFILE_PORTRAIT_240P, 240, 320, 15, 200},
    {VIDEO_PROFILE_PORTRAIT_240P_3, 240, 240, 15, 140},
    {VIDEO_PROFILE_PORTRAIT_240P_4, 240, 424, 15, 220},
    {VIDEO_PROFILE_PORTRAIT_360P, 360, 640, 15, 400},
    {VIDEO_PROFILE_PORTRAIT_360P_3, 360, 360, 15, 260},
    {VIDEO_PROFILE_PORTRAIT_360P_4, 360, 640, 30, 600},
    {VIDEO_PROFILE_PORTRAIT_360P_6, 360, 360, 30, 400},
    {VIDEO_PROFILE_PORTRAIT_360P_7, 360, 480, 15, 320},
    {VIDEO_PROFILE_PORTRAIT_360P_8, 360, 480, 30, 490},
    {VIDEO_PROFILE_PORTRAIT_360P_9, 360, 640, 15, 800},
    {VIDEO_PROFILE_PORTRAIT_360P_10, 360, 640, 24, 800},
    {VIDEO_PROFILE_PORTRAIT_360P_11, 360, 640, 24, 1000},
    {VIDEO_PROFILE_PORTRAIT_480P, 480, 640, 15, 500},
    {VIDEO_PROFILE_PORTRAIT_480P_3, 480, 480, 15, 400},
    {VIDEO_PROFILE_PORTRAIT_480P_4, 480, 640, 30, 750},
    {VIDEO_PROFILE_PORTRAIT_480P_6, 480, 480, 30, 600},
    {VIDEO_PROFILE_PORTRAIT_480P_8, 480, 848, 15, 610},
    {VIDEO_PROFILE_PORTRAIT_480P_9, 480, 848, 30, 930},
    {VIDEO_PROFILE_PORTRAIT_480P_10, 480, 640, 10, 400},
    {VIDEO_PROFILE_PORTRAIT_720P, 720, 1280, 15, 1130},
    {VIDEO_PROFILE_PORTRAIT_720P_3, 720, 1280, 30, 1710},
    {VIDEO_PROFILE_PORTRAIT_720P_5, 720, 960, 15, 910},
    {VIDEO_PROFILE_PORTRAIT_720P_6, 720, 960, 30, 1380},
    {VIDEO_PROFILE_PORTRAIT_1080P, 1080, 1920, 15, 2080},
    {VIDEO_PROFILE_PORTRAIT_1080P_3, 1080, 1920, 30, 3150},
    {VIDEO_PROFILE_PORTRAIT_1080P_5, 1080, 1920, 60, 4780},
    {VIDEO_PROFILE_PORTRAIT_1440P, 1440, 2560, 30, 4850},
    {VIDEO_PROFILE_PORTRAIT_1440P_2, 1440, 2560, 60, 7350},
    {VIDEO_PROFILE_PORTRAIT_4K, 2160, 3840, 30, 8190},
    {VIDEO_PROFILE_PORTRAIT_4K_3, 2160, 3840, 60, 13500},
};

int VideoProfile::getSlotByProfile(int profile) {
  int slot = -1;
  switch (profile) {
    case VIDEO_PROFILE_EX_SLOT:
      slot = 0;
      break;
    case VIDEO_PROFILE_ENGINE_SET_SLOT:
      slot = 1;
      break;
    default:
      slot = -1;
      break;
  }

  return slot;
}

bool VideoProfile::getVideoOptionsByProfile(int profile, bool swapWidthAndHeight,
                                            VideoNetOptions& options) {
  for (int i = 0; i < sizeof(s_videoProfiles) / sizeof(s_videoProfiles[0]); i++) {
    const auto& item = s_videoProfiles[i];
    if (profile == item.profile) {
      if (!swapWidthAndHeight) {
        options.width = item.width;
        options.height = item.height;
      } else {
        options.width = item.height;
        options.height = item.width;
      }
      options.maxFrameRate = item.frameRate;
      options.bitRate = item.bitRate;
      return true;
    }
  }
  return false;
}

int VideoProfile::setVideoProfile(int profile, int width, int height, int frameRate, int bitrate) {
  int index = getSlotByProfile(profile);
  if (index < 0) return -ERR_INVALID_ARGUMENT;
  s_videoProfiles[index].width = width;
  s_videoProfiles[index].height = height;
  s_videoProfiles[index].frameRate = frameRate;
  s_videoProfiles[index].bitRate = bitrate;
  return 0;
}

}  // namespace rtc
}  // namespace agora
