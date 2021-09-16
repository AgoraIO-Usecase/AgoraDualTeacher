//
//  EduBaseTypes.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#if defined(AGORAEDU_EXPORT)
#define AGORA_API extern "C" __declspec(dllexport)
#else
#define AGORA_API extern "C" __declspec(dllimport)
#endif  // AGORARTC_EXPORT

#define AGORA_CALL __cdecl

#include "AgoraRefPtr.h"
#include "ErrorCode.h"

namespace agora {
namespace edu {

static const int kMaxUserUuidSize = 128;
static const int kMaxStreamUuidSize = 128;
static const int kMaxRoomUuidSize = 128;
static const int kMaxRecordIdSize = 128;
static const int kMaxTokenSize = 1024;
static const int kMaxKeySize = 32;
static const int kMaxValSize = 8 * 1024;

using AppId = const char*;
using CustomerId = const char*;
using CustomerCertificate = const char*;

using View = void*;

enum EduRoleType {
  EDU_ROLE_TYPE_INVALID,
  EDU_ROLE_TYPE_ASSISTANT,
  EDU_ROLE_TYPE_TEACHER,
  EDU_ROLE_TYPE_STUDENT
};

enum EduClassroomType {
  EDU_CLASSROOM_TYPE_1V1,
  EDU_CLASSROOM_TYPE_SMALL,
  EDU_CLASSROOM_TYPE_BIG,
  EDU_CLASSROOM_TYPE_BREAKOUT
};

enum EduActionType {
  EDUACTION_TYPE_APPLY,
  EDUACTION_TYPE_INVITATION,
  EDUACTION_TYPE_ACCEPT,
  EDUACTION_TYPE_REJECT
};

enum EduUserStateChangeType { EDU_USER_STATE_CHANGE_TYPE_CHAT };

struct EduError {
  edu::ERROR_CODE_TYPE code;
  const char* message;

  EduError() : code(ERR_FAILED), message(nullptr) {}
  EduError(ERROR_CODE_TYPE err, const char* message)
      : code(err), message(message) {}

  void set(ERROR_CODE_TYPE c, const char* mess) {
    code = c;
    message = mess;
  }
};

#define EDU_ERROR_DEFAULT(err, msg) \
  EduError(static_cast<ERROR_CODE_TYPE>(err), msg)
#define EDU_ERROR_RTC(err, msg) \
  err ? EDU_ERROR_DEFAULT(err + 300, msg) : EDU_ERROR_DEFAULT(ERR_OK, msg)
#define EDU_ERROR_RTM(err, msg) \
  err ? EDU_ERROR_DEFAULT(err + 100, msg) : EDU_ERROR_DEFAULT(ERR_OK, msg)
#define EDU_ERROR_NETWORK(err, msg) \
  err ? EDU_ERROR_DEFAULT(err + 200, msg) : EDU_ERROR_DEFAULT(ERR_OK, msg)

struct Property {
  char key[kMaxKeySize];
  char value[kMaxValSize];

  Property() : key{0}, value{0} {}
};

class IPropertyCollection : public RefCountInterface {
 public:
  virtual size_t NumberOfProperties() = 0;
  virtual bool GetProperty(size_t property_index, Property& property) = 0;

 protected:
  ~IPropertyCollection() {}
};

}  // namespace edu
}  // namespace agora
