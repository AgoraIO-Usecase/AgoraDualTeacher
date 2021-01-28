//
//  Agora Media SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "cache_file_manager.h"

#include <fstream>

#include "base/base_context.h"
#include "utils/tools/util.h"

using namespace agora::commons;

namespace agora {
namespace base {

static const char* const MODULE_NAME = "[CFM]";

std::string CacheFileManager::CacheFileHeader::encode(const std::string& decoded_str) {
  std::string encoded_str(k_header_len_ + decoded_str.size(), 0);

  _Header header;
  header.magic_num = c_magic_num_;
  header.version = c_version_;
  header.length = encoded_str.size();
  header.check_sum = 0;

  std::memcpy(reinterpret_cast<void*>(&encoded_str[0]), &header, k_header_len_);
  std::memcpy(reinterpret_cast<void*>(&encoded_str[k_header_len_]), &decoded_str[0],
              decoded_str.size());

  // update checksum
  *reinterpret_cast<uint64_t*>(&encoded_str[k_check_sum_idx_]) = _CalcCheckSum(encoded_str);

  return encoded_str;
}

std::string CacheFileManager::CacheFileHeader::decode(const std::string& encoded_str) {
  if (!_checkFileHeader(encoded_str)) {
    log(LOG_ERROR, "%s: failed to check file header in decode()", MODULE_NAME);
    return "";
  }

  return encoded_str.substr(k_header_len_, encoded_str.size() - k_header_len_);
}

uint64_t CacheFileManager::CacheFileHeader::_CalcCheckSum(const std::string& encoded_str) {
  uint64_t check_sum = 0;

  for (int32_t i = 0; i < encoded_str.size();) {
    if ((encoded_str.size() - i) < sizeof(uint16_t)) {
      check_sum += *reinterpret_cast<const uint8_t*>(&encoded_str[i]);
      break;
    }

    check_sum += *reinterpret_cast<const uint16_t*>(&encoded_str[i]);
    i += sizeof(uint16_t);

    if (i == k_check_sum_idx_) {
      i += k_check_sum_len_;
    }
  }

  return check_sum;
}

bool CacheFileManager::CacheFileHeader::_checkFileHeader(const std::string& encoded_str) {
  // total length shouldn't be shorter than common file header
  if (encoded_str.size() < k_header_len_) {
    return false;
  }

  _Header header;
  std::memcpy(&header, encoded_str.c_str(), k_header_len_);

  // check magic number, version and length
  if (header.magic_num != c_magic_num_ || header.version != c_version_ ||
      header.length != encoded_str.size()) {
    return false;
  }

  // check 'check sum' finally
  return (header.check_sum == _CalcCheckSum(encoded_str));
}

bool CacheFileManager::flushToFile(CacheType cache_type, const std::string& decoded_str) {
  if (!_ValidCacheType(cache_type)) {
    log(LOG_ERROR, "%s: invalid cache type in flushToFile()", MODULE_NAME);
    return false;
  }

  const char* const magic_str = CacheFileHeader::MagicString(cache_type);
  if (!magic_str) {
    log(LOG_ERROR, "%s: failed to get magic string in flushToFile()", MODULE_NAME);
    return false;
  }

  uint32_t version = CacheFileHeader::HeaderVersion(cache_type);

  CacheFileHeader cache_file_header(magic_str, version);

  std::string encoded_str = cache_file_header.encode(decoded_str);
  if (encoded_str.empty()) {
    log(LOG_ERROR, "%s: empty encoded string in flushToFile()", MODULE_NAME);
    return false;
  }

  std::string file_name =
      (cache_type == CacheType::kNormal ? _getCacheFileName() : _getReportFileName());
  if (file_name.empty()) {
    log(LOG_ERROR, "%s: failed to get file name in flushToFile()", MODULE_NAME);
    return false;
  }

  std::ofstream out_file(file_name, std::ios::trunc | std::ofstream::binary);
  if (!out_file.is_open()) {
    log(LOG_ERROR, "%s: open cache file %s for write failed with error in flushToFile(): %s",
        MODULE_NAME, file_name.c_str(), strerror(errno));
    return false;
  }

  out_file.write(&encoded_str[0], encoded_str.size());
  out_file.close();

  return true;
}

std::string CacheFileManager::loadFromFile(CacheType cache_type) {
  if (!_ValidCacheType(cache_type)) {
    log(LOG_ERROR, "%s: invalid cache type in load()", MODULE_NAME);
    return "";
  }

  std::string file_name =
      (cache_type == CacheType::kNormal ? _getCacheFileName() : _getReportFileName());
  if (file_name.empty()) {
    log(LOG_ERROR, "%s: failed to get file name in loadFromFile()", MODULE_NAME);
    return "";
  }

  std::ifstream in_file(file_name.c_str(), std::ifstream::binary);
  if (!in_file.is_open()) {
    log(LOG_DEBUG, "%s: open cache file %s for read failed with error in loadFromFile(): %s",
        MODULE_NAME, file_name.c_str(), strerror(errno));
    return "";
  }

  std::string encoded_str =
      std::string(std::istreambuf_iterator<char>(in_file), std::istreambuf_iterator<char>());

  in_file.close();

  if (encoded_str.empty()) {
    log(LOG_ERROR, "%s: empty encoded string in loadFromFile()", MODULE_NAME);
    return "";
  }

  const char* const magic_str = CacheFileHeader::MagicString(cache_type);
  if (!magic_str) {
    log(LOG_ERROR, "%s: failed to get magic string in loadFromFile()", MODULE_NAME);
    return "";
  }

  uint32_t version = CacheFileHeader::HeaderVersion(cache_type);

  CacheFileHeader cache_file_header(magic_str, version);

  return cache_file_header.decode(encoded_str);
}

std::string CacheFileManager::_getCacheFileName() const {
  return join_path(context_.getDataDir(), "agorasdk.dat");
}

std::string CacheFileManager::_getReportFileName() const {
  return join_path(context_.getDataDir(), "agorareport.dat");
}

}  // namespace base
}  // namespace agora
