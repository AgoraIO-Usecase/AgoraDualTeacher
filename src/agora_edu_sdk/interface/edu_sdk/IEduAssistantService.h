//
//  EduAssistantService.h
//
//  Created by SRS on 2020/8/25.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "EduClassroom.h"

#include "IEduUserService.h"

namespace agora {
namespace edu {

class IEduAssistantOperationEventHandler : public IEduUserOperationEventHandler {
 public:
  virtual void OnCreateOrUpdateTeacherStreamCompleted(EduStream stream,
                                                      EduError err) = 0;

  virtual void OnCreateOrUpdateStudentStreamCompleted(EduStream stream,
                                                      EduError err) = 0;
};

class IEduAssistantService : public IEduUserService {
 public:
  // Teacher Stream
  virtual EduError CreateOrUpdateTeacherStream(EduStream remote_stream) = 0;

  // Student Stream
  virtual EduError CreateOrUpdateStudentStream(EduStream remote_stream) = 0;

  virtual void RegisterOperationEventHandler(
      IEduAssistantOperationEventHandler* handler) = 0;
  virtual void UnregisterOperationEventHandler(
      IEduAssistantOperationEventHandler* handler) = 0;

};

}  // namespace edu
}  // namespace agora
