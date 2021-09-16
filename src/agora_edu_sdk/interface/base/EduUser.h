//
//  EduUser.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"
#include "EduBaseTypes.h"
#include "EduStream.h"

namespace agora {
namespace edu {

struct EduUser {
  char user_uuid[kMaxUserUuidSize];
  char user_name[kMaxUserUuidSize];
  EduRoleType role;
  bool is_chat_allowed;

  agora_refptr<IPropertyCollection> properties;

  EduUser()
      : user_uuid{0},
        user_name{0},
        role(EDU_ROLE_TYPE_INVALID),
        is_chat_allowed(true) {}
};

struct EduLocalUser : public EduUser {
  const char user_token[kMaxTokenSize];

  EduLocalUser() : EduUser(), user_token{0} {}
};

struct EduUserEvent {
  EduUser modified_user;
  EduBaseUser operator_user;
};

class IUserInfoCollection : public RefCountInterface {
 public:
  virtual size_t NumberOfUserInfo() = 0;
  virtual bool GetUserInfo(size_t user_index, EduUser& user_info) = 0;

 protected:
  ~IUserInfoCollection() {}
};

class IUserEventCollection : public RefCountInterface {
 public:
  virtual size_t NumberOfUserEvent() = 0;
  virtual bool GetUserEvent(size_t user_event_index,
                            EduUserEvent& user_event) = 0;

 protected:
  ~IUserEventCollection() {}
};

}  // namespace edu
}  // namespace agora