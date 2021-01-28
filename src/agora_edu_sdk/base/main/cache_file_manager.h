//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <string>

namespace agora {
namespace base {

class BaseContext;

enum class CacheType { kNormal = 0, kReport, kAll };

// all the code related to CacheFileHeader and CacheFileManager will be guaranteed
// to run on major worker by CacheManager
class CacheFileManager {
 private:
  struct _Header {
    uint32_t magic_num;
    uint32_t version;
    uint64_t length;
    uint64_t check_sum;
  };

  class CacheFileHeader {
   public:
    CacheFileHeader(uint32_t magic_num, uint32_t version)
        : c_magic_num_(magic_num), c_version_(version) {}

    CacheFileHeader(const char* magic_str, uint32_t version)
        : CacheFileHeader(_CalcMagicNum(magic_str), version) {}

    ~CacheFileHeader() = default;

    std::string encode(const std::string& decoded_str);
    std::string decode(const std::string& encoded_str);

    static const char* MagicString(CacheType cache_type) {
      return (cache_type == CacheType::kNormal ? "ACFM" : "ARFM");
    }

    static uint32_t HeaderVersion(CacheType cache_type) {
      return (cache_type == CacheType::kNormal ? k_cache_ver_ : k_report_ver_);
    }

   private:
    static uint32_t _CalcMagicNum(const char* magic_str) {
      if (!magic_str) {
        return 0;
      }

      return (magic_str[0] + (magic_str[1] << 8) + (magic_str[2] << 16) + (magic_str[3] << 24));
    }

    static uint64_t _CalcCheckSum(const std::string& encoded_str);

    bool _checkFileHeader(const std::string& encoded_str);

   private:
    const uint32_t c_magic_num_;
    const uint32_t c_version_;

    static const uint32_t k_header_len_ = sizeof(_Header);
    static const uint32_t k_check_sum_len_ = sizeof(_Header().check_sum);
    static const uint32_t k_check_sum_idx_ = k_header_len_ - k_check_sum_len_;

    static const uint32_t k_cache_ver_ = 1;
    static const uint32_t k_report_ver_ = 1;
  };

 public:
  explicit CacheFileManager(const BaseContext& context) : context_(context) {}

  ~CacheFileManager() = default;

  bool flushToFile(CacheType cache_type, const std::string& decoded_str);
  std::string loadFromFile(CacheType cache_type);

 private:
  static bool _ValidCacheType(CacheType cache_type) {
    return (cache_type == CacheType::kNormal || cache_type == CacheType::kReport);
  }

  std::string _getCacheFileName() const;
  std::string _getReportFileName() const;

 private:
  const BaseContext& context_;
};

}  // namespace base
}  // namespace agora
