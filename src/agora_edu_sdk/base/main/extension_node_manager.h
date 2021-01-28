//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019.12.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "AgoraRefPtr.h"

namespace agora {
namespace rtc {

template <typename T>
class ExtensionNodeContainer {
 public:
  static std::unique_ptr<ExtensionNodeContainer<T>> Create() {
    return std::make_unique<ExtensionNodeContainer<T>>();
  }

 public:
  ExtensionNodeContainer() = default;
  ~ExtensionNodeContainer() {
    std::lock_guard<std::mutex> _(lock_);
    nodes_.clear();
    node_keys_.clear();
  }

  void AddNode(const char* name, const char* vendor, agora_refptr<T> node) {
    if (!name || !*name || !vendor || !*vendor || !node) return;
    std::string key = std::string(name) + "." + vendor;
    std::lock_guard<std::mutex> _(lock_);
    if (node_keys_.find(key) != node_keys_.end()) return;
    node_keys_[key] = nodes_.size();
    nodes_.emplace_back(node);
  }

  void RemoveNode(const char* name, const char* vendor) {
    if (!name || !*name || !vendor || !*vendor) return;
    std::string key = std::string(name) + "." + vendor;
    std::lock_guard<std::mutex> _(lock_);
    if (node_keys_.find(key) == node_keys_.end()) return;
    int idx = node_keys_[key];
    node_keys_.erase(key);

    // remote n'th element
    auto itor = nodes_.begin();
    std::advance(itor, idx);
    nodes_.erase(itor);
  }

  agora_refptr<T> Get(const char* name, const char* vendor) {
    if (!name || !*name || !vendor || !*vendor) return nullptr;
    std::string key = std::string(name) + "." + vendor;
    std::lock_guard<std::mutex> _(lock_);
    if (node_keys_.find(key) == node_keys_.end()) return nullptr;
    int idx = node_keys_[key];
    auto itor = nodes_.begin();
    std::advance(itor, idx);
    return *itor;
  }

  std::list<agora_refptr<T>> Clone() {
    std::lock_guard<std::mutex> _(lock_);
    return nodes_;
  }

 private:
  std::mutex lock_;
  std::unordered_map<std::string, int> node_keys_;
  std::list<agora_refptr<T>> nodes_;
};

class ExtensionNodes {
 public:
  ExtensionNodes();

  ~ExtensionNodes();

  int CreateVideoFilter(const char* name, const char* vendor);

  int DestroyVideoFilter(const char* name, const char* vendor);

  int AddVideoFilter(const char* name, const char* vendor, agora_refptr<IVideoFilter> filter);

  std::list<agora_refptr<IVideoFilter>> GetVideoFilters();

  agora_refptr<IVideoFilter> GetVideoFilter(const char* name, const char* vendor);

 private:
  std::unique_ptr<ExtensionNodeContainer<IVideoFilter>> video_filters_;
};

}  // namespace rtc
}  // namespace agora
