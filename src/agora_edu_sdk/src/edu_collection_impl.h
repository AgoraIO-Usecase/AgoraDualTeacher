//  edu_collection_impl.h
//
//  Created by WQX on 2020/11/19.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "interface/base/EduUser.h"

#include <algorithm>
#include <vector>

namespace agora {
namespace edu {

class StreamInfoCollection : public IStreamInfoCollection {
 public:
  StreamInfoCollection() = default;

  size_t NumberOfStreamInfo() override { return stream_infos_.size(); }

  bool GetStreamInfo(size_t stream_index, EduStream& stream_info) override {
    if (stream_index >= 0 && stream_index < stream_infos_.size()) {
      stream_info = stream_infos_[stream_index];
      return true;
    }
    return false;
  }

  void AddStreamInfo(const EduStream& stream_info) {
    stream_infos_.push_back(stream_info);
  }

  void SetStreamInfo(size_t stream_index, const EduStream& stream_info) {
    if (stream_index < stream_infos_.size())
      stream_infos_[stream_index] = stream_info;
  }

  size_t ExistStream(const char* stream_id) override {
    for (size_t i = 0; i < stream_infos_.size(); ++i) {
      if (strncmp(stream_id, stream_infos_[i].stream_uuid,
                  kMaxStreamUuidSize) == 0)
        return i;
    }
    return -1;
  }

 private:
  std::vector<EduStream> stream_infos_;
};

class UserInfoCollection : public IUserInfoCollection {
 public:
  UserInfoCollection() = default;

  size_t NumberOfUserInfo() override { return user_infos_.size(); }

  bool GetUserInfo(size_t user_event_index, EduUser& user_info) override {
    if (user_event_index >= 0 && user_event_index < user_infos_.size()) {
      user_info = user_infos_[user_event_index];
      return true;
    }
    return false;
  }

  void AddUserInfo(const EduUser& user_info) {
    user_infos_.push_back(user_info);
  }

 private:
  std::vector<EduUser> user_infos_;
};

class StreamEventCollection : public IStreamEventCollection {
 public:
  StreamEventCollection() = default;

  size_t NumberOfStreamEvent() override { return stream_events_.size(); }

  bool GetStreamEvent(size_t stream_event_index,
                      EduStreamEvent& stream_event) override {
    if (stream_event_index >= 0 && stream_event_index < stream_events_.size()) {
      stream_event = stream_events_[stream_event_index];
      return true;
    }
    return false;
  }

  void AddStreamEvent(const EduStreamEvent& stream_event) {
    stream_events_.push_back(stream_event);
  }

 private:
  std::vector<EduStreamEvent> stream_events_;
};

class UserEventCollection : public IUserEventCollection {
 public:
  UserEventCollection() = default;

  size_t NumberOfUserEvent() override { return user_events_.size(); }

  bool GetUserEvent(size_t user_event_index,
                    EduUserEvent& user_event) override {
    if (user_event_index >= 0 && user_event_index < user_events_.size()) {
      user_event = user_events_[user_event_index];
      return true;
    }
    return false;
  }

  void AddUserEvent(const EduUserEvent& user_event) {
    user_events_.push_back(user_event);
  }

 private:
  std::vector<EduUserEvent> user_events_;
};

class PropertyCollection : public IPropertyCollection {
 public:
  PropertyCollection() = default;

  size_t NumberOfProperties() override { return properties_.size(); }
  bool GetProperty(size_t property_index, Property& property) override {
    if (property_index >= 0 && property_index < properties_.size()) {
      property = properties_[property_index];
      return true;
    }
    return false;
  }

  void AddProperty(const Property& property) {
    properties_.push_back(property);
  }

  void AddProperty(const std::pair<std::string, std::string>& property) {
    bool is_find = false;
    for (auto& property_ : properties_) {
      if (strncmp(property_.key, property.first.c_str(), kMaxKeySize) == 0) {
        is_find = true;
        strncpy(property_.value, property.second.c_str(), kMaxKeySize);
      }
    }

    if (!is_find) {
      Property pro;
      strncpy(pro.key, property.first.c_str(), kMaxKeySize);
      strncpy(pro.value, property.second.c_str(), kMaxKeySize);
      this->AddProperty(pro);
    }
  }

  void RemoveProperty(const Property& remove_property) {
    properties_.erase(std::remove_if(properties_.begin(), properties_.end(),
                                     [&remove_property](Property& property) {
                                       return strcmp(property.key,
                                                     remove_property.key) == 0;
                                     }),
                      properties_.end());
  }

  void RemoveProperty(
      const std::pair<std::string, std::string>& remove_property) {
    properties_.erase(
        std::remove_if(properties_.begin(), properties_.end(),
                       [&remove_property](Property& property) {
                         return strcmp(property.key,
                                       remove_property.first.c_str()) == 0;
                       }),
        properties_.end());
  }

  void ClearProperties() { properties_.clear(); }

 protected:
  ~PropertyCollection() {}

 private:
  std::vector<Property> properties_;
};

}  // namespace edu
}  // namespace agora