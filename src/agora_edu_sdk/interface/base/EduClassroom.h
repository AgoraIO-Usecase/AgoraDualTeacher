//
//  EduClassroom.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "EduBaseTypes.h"

namespace agora {
namespace edu {

struct EduClassroomInfo {
  // 客户提供roomUuid
  char room_uuid[kMaxRoomUuidSize];
  char room_name[kMaxRoomUuidSize];

  EduClassroomInfo() : room_uuid{0}, room_name{0} {}
};

enum EduCourseState { EDU_COURSE_STATE_STOP, EDU_COURSE_STATE_START };

struct EduClassroomStatus {
  EduCourseState course_state;
  long start_time;
  bool is_student_chat_allowed;
  int online_users_count;

  EduClassroomStatus()
      : course_state(EDU_COURSE_STATE_STOP),
        start_time(0),
        is_student_chat_allowed(false),
        online_users_count(0) {}
};

struct EduClassroom {
  EduClassroomInfo room_info;
  EduClassroomStatus room_status;

  agora_refptr<IPropertyCollection> room_properties;
};

}  // namespace edu
}  // namespace agora