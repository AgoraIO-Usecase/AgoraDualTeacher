//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#import <Foundation/Foundation.h>

#include "view_manager.h"

namespace agora {
namespace rtc {

utils::object_handle ViewToHandle(view_t view) {
  NSObject* obj = (__bridge NSObject*)(view);
  return utils::ObjectToHandle(obj);
}

NSObject* HandleToView(utils::object_handle handle) { return utils::HandleToNSObject(handle); }

}  // namespace rtc
}  // namespace agora