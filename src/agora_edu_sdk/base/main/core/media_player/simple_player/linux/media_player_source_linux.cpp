//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_source_linux.h"

#include "utils/log/log.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSL]";

MediaPlayerSourceLinux::MediaPlayerSourceLinux(base::IAgoraService* agora_service,
                                               utils::worker_type player_worker)
    : MediaPlayerSourceImpl(agora_service, player_worker) {}

MediaPlayerSourceLinux::~MediaPlayerSourceLinux() {}

bool MediaPlayerSourceLinux::doOpen(const char* url, int64_t start_pos) { return false; }

void MediaPlayerSourceLinux::doPlay() {}

void MediaPlayerSourceLinux::doPause() {}

bool MediaPlayerSourceLinux::doStop() { return false; }

void MediaPlayerSourceLinux::doResume() {}

void MediaPlayerSourceLinux::doSeek(int64_t new_pos_ms) {}

void MediaPlayerSourceLinux::doGetDuration(int64_t& dur_ms) {}

void MediaPlayerSourceLinux::doGetPlayPosition(int64_t& curr_pos_ms) {}

void MediaPlayerSourceLinux::doGetStreamCount(int64_t& count) {}

void MediaPlayerSourceLinux::doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {}

} /* namespace rtc */
} /* namespace agora */
