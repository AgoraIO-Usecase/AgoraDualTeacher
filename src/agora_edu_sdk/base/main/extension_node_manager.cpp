//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2019.12.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "extension_node_manager.h"

#include "utils/log/log.h"
#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif
#include "main/core/extension_control_impl.h"

namespace agora {
namespace rtc {

ExtensionNodes::ExtensionNodes() : video_filters_(ExtensionNodeContainer<IVideoFilter>::Create()) {}

ExtensionNodes::~ExtensionNodes() { video_filters_.reset(); }

int ExtensionNodes::CreateVideoFilter(const char* name, const char* vendor) {
#if defined(HAS_BUILTIN_EXTENSIONS)
  if (!vendor || !*vendor) vendor = BUILTIN_EXTENSION_PROVIDER;
#else
  if (!vendor || !*vendor) return -ERR_FAILED;
#endif
  auto ex_control = ExtensionControlImpl::GetInstance();
  if (!ex_control) {
    return -ERR_INVALID_STATE;
  }
  auto provider = ex_control->getExtensionProvider(vendor);
  if (!provider) return -ERR_FAILED;
  auto filter = provider->createVideoFilter(name);
  if (!filter) return -ERR_FAILED;
  video_filters_->AddNode(name, vendor, filter);
  return ERR_OK;
}

int ExtensionNodes::DestroyVideoFilter(const char* name, const char* vendor) {
#if defined(HAS_BUILTIN_EXTENSIONS)
  if (!vendor || !*vendor) vendor = BUILTIN_EXTENSION_PROVIDER;
#else
  if (!vendor || !*vendor) return -ERR_FAILED;
#endif
  video_filters_->RemoveNode(name, vendor);
  return ERR_OK;
}

int ExtensionNodes::AddVideoFilter(const char* name, const char* vendor,
                                   agora_refptr<IVideoFilter> filter) {
  if (!filter) {
    return -ERR_INVALID_ARGUMENT;
  }
#if defined(HAS_BUILTIN_EXTENSIONS)
  if (!vendor || !*vendor) vendor = BUILTIN_EXTENSION_PROVIDER;
#else
  if (!vendor || !*vendor) return -ERR_FAILED;
#endif
  video_filters_->AddNode(name, vendor, filter);
  return ERR_OK;
}

std::list<agora::agora_refptr<agora::rtc::IVideoFilter>> ExtensionNodes::GetVideoFilters() {
  return video_filters_->Clone();
}

agora::agora_refptr<agora::rtc::IVideoFilter> ExtensionNodes::GetVideoFilter(const char* name,
                                                                             const char* vendor) {
#if defined(HAS_BUILTIN_EXTENSIONS)
  if (!vendor || !*vendor) vendor = BUILTIN_EXTENSION_PROVIDER;
#else
  if (!vendor || !*vendor) return nullptr;
#endif
  auto filter = video_filters_->Get(name, vendor);
  return filter;
}

}  // namespace rtc
}  // namespace agora
