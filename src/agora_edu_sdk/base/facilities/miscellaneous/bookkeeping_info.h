//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <memory>
#include <string>

#include "utils/storage/storage.h"

namespace agora {
namespace rtc {

//
// BookkeepingInfo class will record some useful info into database
// Stats fetch from BookkeepingInfo is calculated from all data of the entire lifecycle of our SDK
// ( from dll install to dll uninstall )
//
class BookkeepingInfo {
 public:
  static std::unique_ptr<BookkeepingInfo> Create(std::shared_ptr<utils::Storage> storage);

 public:
  ~BookkeepingInfo();

 public:
  void OnServiceInitialize();

  int64_t SwapInitializeCount(int64_t to_value = 0);

  int64_t SwapCrashCount(int64_t to_value = 0);

 private:
  explicit BookkeepingInfo(std::shared_ptr<utils::Storage> storage);

  void AddNumber(const char* key, int64_t value);

  int64_t Swap(const char* key, int64_t value);

 private:
  std::shared_ptr<utils::Storage> storage_;
};

}  // namespace rtc
}  // namespace agora
