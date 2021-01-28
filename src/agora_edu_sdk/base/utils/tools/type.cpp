//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/packer/packer_type.h"
#include "utils/tools/util.h"
using namespace agora::commons;

std::string isp_service_address::to_string() const {
  return "isp: " + isp + " address: " + address_to_string(ip, port);
}
