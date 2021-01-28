//  edu_collection_impl.cpp
//
//  Created by WQX on 2020/11/19.
//  Copyright © 2020 agora. All rights reserved.
//

#include "interface/base/EduMessage.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include <string>
#include <vector>

namespace agora {
namespace edu {

class AgoraEduMessage : public IAgoraEduMessage {
 public:
  bool SetEduMessage(const char* msg) override {
    if (utils::IsNullOrEmpty(msg)) {
      return false;
    }

    edu_msg_ = msg;

    return true;
  };

  const char* GetEduMessage() const override { return edu_msg_.c_str(); }

  void SetTimestamp(uint64_t ts) override { time_stamp_ = ts; }

  uint64_t GetTimestamp() override { return time_stamp_; }

 private:
  std::string edu_msg_;
  uint64_t time_stamp_ = 0;
};

}  // namespace edu
}  // namespace agora