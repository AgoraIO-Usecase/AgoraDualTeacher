//
//  EduStream.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "AgoraRefPtr.h"
#include "EduBaseTypes.h"

namespace agora {
namespace edu {

enum EduVideoSourceType {
  EDU_VIDEO_SOURCE_TYPE_NONE,
  EDU_VIDEO_SOURCE_TYPE_CAMERA,
  EDU_VIDEO_SOURCE_TYPE_SCREEN
};

struct EduBaseUser {
  char user_uuid[kMaxUserUuidSize];
  char user_name[kMaxUserUuidSize];
  EduRoleType role;

  EduBaseUser() : user_uuid{0}, user_name{0}, role(EDU_ROLE_TYPE_INVALID) {}
};

struct EduStream {
  // 原来的streamId隐藏，后台生成
  char stream_uuid[kMaxStreamUuidSize];
  char stream_name[kMaxStreamUuidSize];
  EduVideoSourceType source_type;
  bool has_video;
  bool has_audio;

  EduBaseUser user_info;

  EduStream()
      : stream_uuid{0},
        stream_name{0},
        source_type(EDU_VIDEO_SOURCE_TYPE_NONE),
        has_video(false),
        has_audio(false) {}
};

struct EduStreamEvent {
  EduStream modified_stream;
  EduBaseUser operator_user;
};

class IStreamInfoCollection : public RefCountInterface {
 public:
  virtual size_t NumberOfStreamInfo() = 0;
  virtual bool GetStreamInfo(size_t stream_index, EduStream& stream_info) = 0;
  virtual size_t ExistStream(const char* stream_id) = 0;

 protected:
  ~IStreamInfoCollection() {}
};

class IStreamEventCollection : public RefCountInterface {
 public:
  virtual size_t NumberOfStreamEvent() = 0;
  virtual bool GetStreamEvent(size_t stream_event_index,
                              EduStreamEvent& stream_event) = 0;

 protected:
  ~IStreamEventCollection() {}
};

}  // namespace edu
}  // namespace agora