//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.8
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include <cstring>
#include <algorithm>
#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "base/IAgoraParameter.h"
#include "base/agora_base.h"
#include "base/agora_parameter.h"

// TODO error codes for C API like in agora base
#define ERR_INVALID_ARGUMENT 2

// check BUILD.gn, we should not add more include dirs like utils
static bool isNullOrEmpty(const char* s) {
  return !(s && *s);
}

static void copyString(char* value, uint32_t* value_size, agora::util::AString& str_val) {
    if (!value) {
    *value_size = str_val->length() + 1;
  } else {
    auto copy_size = std::min((size_t)*value_size, str_val->length());
    memcpy(value, str_val->c_str(), copy_size);
    value[copy_size] = '\0';
  }
}

AGORA_API_C_INT agora_parameter_set_int(AGORA_HANDLE agora_parameter, const char* key, int value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);

  return agora_parameter_holder->setInt(key, value);
}

AGORA_API_C_INT agora_parameter_set_uint(AGORA_HANDLE agora_parameter, const char* key,
                                         unsigned int value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);

  return agora_parameter_holder->setUInt(key, value);
}

AGORA_API_C_INT agora_parameter_set_number(AGORA_HANDLE agora_parameter, const char* key,
                                           double value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);

  return agora_parameter_holder->setNumber(key, value);
}

AGORA_API_C_INT agora_parameter_set_string(AGORA_HANDLE agora_parameter, const char* key,
                                           const char* value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);

  return agora_parameter_holder->setString(key, value);
}

AGORA_API_C_INT agora_parameter_set_array(AGORA_HANDLE agora_parameter, const char* key,
                                          const char* json_src) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);

  return agora_parameter_holder->setArray(key, json_src);
}

AGORA_API_C_INT agora_parameter_set_parameters(AGORA_HANDLE agora_parameter,
                                               const char* json_src) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);
  return agora_parameter_holder->setParameters(json_src);
}

AGORA_API_C_INT agora_parameter_get_int(AGORA_HANDLE agora_parameter, const char* key, int* value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);
  int ret = 0;
  int& value_ret = ret;
  int res = agora_parameter_holder->getInt(key, value_ret);
  *value = ret;
  return res;
}

AGORA_API_C_INT agora_parameter_get_uint(AGORA_HANDLE agora_parameter, const char* key,
                                         unsigned int* value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);
  unsigned int ret = 0;
  unsigned int& value_ret = ret;
  int res = agora_parameter_holder->getUInt(key, value_ret);
  *value = ret;
  return res;
}

AGORA_API_C_INT agora_parameter_get_number(AGORA_HANDLE agora_parameter, const char* key,
                                           double* value) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (isNullOrEmpty(key)) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_parameter_holder, agora::base::IAgoraParameter, agora_parameter);
  double ret = 0;
  double& value_ret = ret;
  int res = agora_parameter_holder->getNumber(key, value_ret);
  *value = ret;
  return res;
}

AGORA_API_C_INT agora_parameter_get_string(AGORA_HANDLE agora_parameter, const char* key,
                                           char* value, uint32_t* value_size) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (isNullOrEmpty(key)) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (!value_size) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_param_holder, agora::base::IAgoraParameter, agora_parameter);
  agora::util::AString str_val;
  auto ret = agora_param_holder->getString(key, str_val);
  if (ret != 0) {
    return ret;
  }

  copyString(value, value_size, str_val);
  return 0;
}

AGORA_API_C_INT agora_parameter_get_array(AGORA_HANDLE agora_parameter, const char* key, const char* json_src,
                                          char* value, uint32_t* value_size) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (isNullOrEmpty(key)) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (!value_size) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_param_holder, agora::base::IAgoraParameter, agora_parameter);
  agora::util::AString str_val;
  auto ret = agora_param_holder->getArray(key, json_src, str_val);
  if (ret != 0) {
    return ret;
  }

  copyString(value, value_size, str_val);
  return 0;
}

AGORA_API_C_INT agora_parameter_get_object(AGORA_HANDLE agora_parameter, const char* key,
                                           char* value, uint32_t* value_size) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (isNullOrEmpty(key)) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (!value_size) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_param_holder, agora::base::IAgoraParameter, agora_parameter);
  agora::util::AString str_val;
  auto ret = agora_param_holder->getObject(key, str_val);
  if (ret != 0) {
    return ret;
  }

  copyString(value, value_size, str_val);
  return 0;
}

AGORA_API_C_INT agora_parameter_convert_path(AGORA_HANDLE agora_parameter, const char* file_path,
                                             char* value, uint32_t* value_size) {
  if (!agora_parameter) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (isNullOrEmpty(file_path)) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (!value_size) {
    return -ERR_INVALID_ARGUMENT;
  }

  REINTER_CAST(agora_param_holder, agora::base::IAgoraParameter, agora_parameter);
  agora::util::AString str_val;
  auto ret = agora_param_holder->convertPath(file_path, str_val);
  if (ret != 0) {
    return ret;
  }

  copyString(value, value_size, str_val);
  return 0;
}
