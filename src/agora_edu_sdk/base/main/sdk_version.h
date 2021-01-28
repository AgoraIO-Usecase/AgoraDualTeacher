//  NOLINT
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
/*
   Build version of the binary,
   should be passed by CI for formal release version
 */
#if !defined(AGORA_SDK_BUILD_NUMBER)
#define AGORA_SDK_BUILD_NUMBER "0"
#else
#pragma message("CI assign build number: " AGORA_SDK_BUILD_NUMBER)
#endif

/* Version number of package */
#if !defined(AGORA_SDK_VERSION)
#define AGORA_SDK_VERSION "2.7.0"
#else
#pragma message("CI assign sdk version number: " AGORA_SDK_VERSION)
#endif

#if !defined(AGORA_SDK_PRODUCT_PROFILE)
#define AGORA_SDK_PRODUCT_PROFILE ""
#else
#pragma message("CI assign product profile: " AGORA_SDK_PRODUCT_PROFILE)
#endif

#define AGORA_SDK_VERSION_MAJOR 2
#define AGORA_SDK_VERSION_MINOR 7
#define AGORA_SDK_VERSION_HOTFIX 0
#define AGORA_SDK_VERSION_PATCH 0
#define AGORA_SDK_VERSION_EXTRA "ivan"
