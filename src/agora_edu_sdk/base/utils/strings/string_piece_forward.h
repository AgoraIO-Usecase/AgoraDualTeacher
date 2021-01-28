//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//
#pragma once

#include <string>

namespace agora {
namespace utils {

template <typename STRING_TYPE>
class BasicStringPiece;
typedef BasicStringPiece<std::string> StringPiece;
typedef BasicStringPiece<std::wstring> WStringPiece;

}  // namespace utils
}  // namespace agora
