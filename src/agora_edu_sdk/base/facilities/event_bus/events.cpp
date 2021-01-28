//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "events.h"

namespace agora {
namespace utils {

const EventId AudioDeviceEvent::ID = EventId::AudioDeviceEvent;
const EventId VideoDeviceEvent::ID = EventId::VideoDeviceEvent;
const EventId VideoFrameEvent::ID = EventId::VideoFrameEvent;
const EventId TwoBytesCapEvent::ID = EventId::TwoBytesCapEvent;
const EventId SystemErrorEvent::ID = EventId::SystemErrorEvent;
const EventId TargetBitrateEvent::ID = EventId::TargetBitrateEvent;
const EventId TargetMinMaxBitrateEvent::ID = EventId::TargetMinMaxBitrateEvent;

}  // namespace utils
}  // namespace agora
