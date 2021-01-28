//
//  EduStudentService.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "IEduUserService.h"

namespace agora {
namespace edu {

class IEduStudentOperationEventHandler : public IEduUserOperationEventHandler {
};

class IEduStudentService : public IEduUserService {
 public:

  virtual void RegisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) = 0;
  virtual void UnregisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) = 0;
};

}  // namespace edu
}  // namespace agora