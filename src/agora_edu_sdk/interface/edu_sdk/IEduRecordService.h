//
//  EduRecordService.h
//  Demo
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include "interface/base/EduBaseTypes.h" 

namespace rte {
namespace edu {

enum RecordState {
  Record_State_Recording,
  Record_State_Finished,
  Record_State_Wait_Download,
  Record_State_Wait_Convert,
  Record_State_Wait_Upload
};

struct EduRecordInfo {
  AppId app_id;
  char room_uuid[kMaxRoomUuidSize];
  char record_id[kMaxRecordIdSize];
  RecordState record_state;
  long recording_time;
};

class EduRecordEventHandler {
 public:
  virtual void OnStartRecord(EduRecordInfo record) = 0;
  virtual void OnStopRecord(EduRecordInfo record) = 0;
};

class EduRecordService {
 public:
  virtual EduRecordInfo GetRecordInfo() = 0;

  virtual EduError StartRecord() = 0;
  virtual EduError StopRecord() = 0;

  virtual void RegisterEventHandler(EduRecordEventHandler *handler) = 0;
  virtual void UnregisterEventHandler(EduRecordEventHandler *handler) = 0;
};

}  // namespace edu
}  // namespace rte