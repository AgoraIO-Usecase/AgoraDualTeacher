//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/object/object_table.h"

#include <cstdint>
#include <ctime>
#include "utils/tools/util.h"
#include "utils/thread/thread_pool.h"


namespace agora {
namespace utils {

static_assert(sizeof(object_handle) == sizeof(void*), "sizeof(object_handle) != sizeof(void *)!");

ObjectTable::ObjectTable() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    gc_worker_ = utils::minor_worker("AgoraObjMgrWorker");
    gc_timer_.reset(gc_worker_->createTimer([this] { GC(); }, 60 * 1000));
    return 0;
  });
}

ObjectTable::~ObjectTable() {
  utils::major_worker()->sync_call(LOCATION_HERE, [this] {
    gc_timer_->cancel();
    gc_timer_.reset();
    gc_worker_.reset();
    return 0;
  });
}

ObjectStrongRef ObjectTable::HandleToObject(object_handle handle) {
  if (handle == kInvalidHandle) return nullptr;

  std::lock_guard<std::mutex> _(lock_);

  if (handle_to_obj_map_.find(handle) == handle_to_obj_map_.end()) {
    return nullptr;
  }

  ObjectWeakRef& weak = handle_to_obj_map_[handle];
  if (!weak) return nullptr;

  auto shared = weak->Lock();
  if (!shared) {
    EraseHandle(handle);
  }

  return shared;
}

object_handle ObjectTable::ObjectToHandle(ObjectWeakRef obj) {
  if (!obj) return kInvalidHandle;

  auto id = obj->ID();

  std::lock_guard<std::mutex> _(lock_);

  object_handle handle = kInvalidHandle;

  if (id_to_handle_map_.find(id) != id_to_handle_map_.end()) {
    handle = id_to_handle_map_[id];
  }

  if (handle == kInvalidHandle) {
    handle = CalculateHandleValue();
  }

  InsertHandle(handle, id, std::move(obj));

  return handle;
}

void ObjectTable::GC() {
  std::vector<object_handle> handles;
  {
    std::lock_guard<std::mutex> _(lock_);
    for (auto& pair : handle_to_obj_map_) {
      handles.push_back(pair.first);
    }
  }

  // try to fetch objects
  // and this will collect garbage
  for (auto& handle : handles) {
    auto shared = HandleToObject(handle);
  }
}

void ObjectTable::EraseHandle(object_handle handle) {
  handle_to_obj_map_.erase(handle);

  if (handle_to_id_map_.find(handle) != handle_to_id_map_.end()) {
    auto id = handle_to_id_map_[handle];
    id_to_handle_map_.erase(id);
  }

  handle_to_id_map_.erase(handle);
}

void ObjectTable::InsertHandle(object_handle handle, const void* key, ObjectWeakRef weak) {
  handle_to_obj_map_[handle] = std::move(weak);
  id_to_handle_map_[key] = handle;
  handle_to_id_map_[handle] = key;
}

object_handle ObjectTable::CalculateHandleValue() {
  // we believe we can always get a unused 64 bit id
  uint64_t max = UINT64_MAX;
  switch (sizeof(object_handle)) {
    case sizeof(uint8_t):
      max = UINT8_MAX;
      break;
    case sizeof(uint16_t):
      max = UINT16_MAX;
      break;
    case sizeof(uint32_t):
      max = UINT32_MAX;
      break;
    case sizeof(uint64_t):
      max = UINT64_MAX;
      break;
    default:
      break;
  }

  if (handle_to_obj_map_.size() == max) {
    // so not possible because we'are not going to support 16 bit (or 8 bit ...) os
    // 32 bit os has enough room for object table index, not to say 64 bit
    return kInvalidHandle;
  }

  for (;;) {
    object_handle id = commons::getUniformRandomNum(2, max);
    if (id == kInvalidHandle) continue;
    if (handle_to_obj_map_.find(id) == handle_to_obj_map_.end()) return id;
  }
}

void ObjectTableGC() { GetUtilGlobal()->object_table->GC(); }

}  // namespace utils
}  // namespace agora
