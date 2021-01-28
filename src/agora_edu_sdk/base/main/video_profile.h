//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>

namespace agora {
namespace rtc {
	struct VideoNetOptions;

class VideoProfile
{
public:
    static bool getVideoOptionsByProfile(int profile, bool swapWidthAndHeight, VideoNetOptions& options);
	static int setVideoProfile(int profile, int width, int height, int frameRate, int bitrate);
private:
    static int getSlotByProfile(int profile);
};

}}
