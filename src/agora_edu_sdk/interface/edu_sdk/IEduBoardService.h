//
//  EduBoardService.h
//  Demo
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "interface/base/EduBaseTypes.h"
#include "interface/base/EduUser.h"

namespace agora {
namespace edu {

struct EduBoardOperator {
  bool is_publisher;
  EduUser user;

  EduBoardOperator() : is_publisher(false) {}
};

struct EduBoardRoom {
  bool board_follow;
  EduBoardOperator *board_operators;
  size_t count;

  EduBoardRoom() : board_follow(false), board_operators(nullptr), count(0) {}
};

class IEduBoardEventHandler {
 public:
  virtual void OnFollowMode(bool enable, EduUser operator_user) = 0;
  virtual void OnPermissionGranted(EduUser student, EduUser operator_user) = 0;
  virtual void OnPermissionRevoked(EduUser student, EduUser operator_user) = 0;
};

class IEduBoardService {
 public:
  virtual EduBoardRoom GetBoardRoom() = 0;
  virtual EduError FollowMode(bool enable) = 0;
  virtual EduError GrantPermission(const EduUser &remote_user) = 0;
  virtual EduError RevokePermission(const EduUser &remote_user) = 0;
  virtual EduError GetBoardInfo(char *white_info) = 0;

  virtual void RegisterEventHandler(IEduBoardEventHandler *handler) = 0;
  virtual void UnregisterEventHandler(IEduBoardEventHandler *handler) = 0;

 protected:
  virtual ~IEduBoardService() {}
};

}  // namespace edu
}  // namespace agora
