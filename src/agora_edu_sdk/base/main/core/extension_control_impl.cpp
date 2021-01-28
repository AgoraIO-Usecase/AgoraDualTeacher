//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "extension_control_impl.h"

#include <fstream>
#include <mutex>

#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"
#include "video_frame_buffer/external_video_frame.h"
#include "video_frame_buffer/global_buffer_pool.h"

namespace {
const char* MODULE_NAME = "[Extension]";

agora::rtc::ExtensionControlImpl* g_instance = nullptr;
std::mutex instance_lock;
}  // namespace

namespace agora {
namespace rtc {

ExtensionControlImpl* ExtensionControlImpl::Create() {
  std::lock_guard<std::mutex> _(instance_lock);
  if (!g_instance) {
    g_instance = new ExtensionControlImpl;
  }
  return g_instance;
}

void ExtensionControlImpl::Destroy(ExtensionControlImpl* p) {
  std::lock_guard<std::mutex> _(instance_lock);
  if (p != g_instance) {
    assert(0);  // We should never reach here
  }
  delete g_instance;
  g_instance = nullptr;
}

ExtensionControlImpl* ExtensionControlImpl::GetInstance() {
  std::lock_guard<std::mutex> _(instance_lock);
  return g_instance;
}

agora_refptr<IExtensionProvider> ExtensionControlImpl::getExtensionProvider(
    const char* vendor_name) {
  if (!vendor_name || !*vendor_name) {
    return nullptr;
  }
  agora_refptr<IExtensionProvider> result = nullptr;
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, vendor_name, &result] {
    if (providers_.find(vendor_name) == providers_.end()) {
      return 0;
    }
    result = providers_[vendor_name];
    return 0;
  });
  return result;
}

void ExtensionControlImpl::getCapabilities(Capabilities& capabilities) {
  capabilities.audio = true;
  capabilities.video = false;
#ifdef FEATURE_VIDEO
  capabilities.video = true;
#endif  // FEATURE_VIDEO
}

int ExtensionControlImpl::registerExtensionProvider(
    const char* vendor_name, agora::agora_refptr<IExtensionProvider> provider) {
  if (!vendor_name || !*vendor_name || !provider) {
    return -ERR_INVALID_ARGUMENT;
  }
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, vendor_name, provider] {
    if (providers_.find(vendor_name) != providers_.end()) {
      return -ERR_ALREADY_IN_USE;
    }
    providers_[vendor_name] = provider;
    return 0;
  });
}

int ExtensionControlImpl::unregisterExtensionProvider(const char* vendor_name) {
  if (!vendor_name || !*vendor_name) {
    return -ERR_INVALID_ARGUMENT;
  }
  return utils::major_worker()->sync_call(LOCATION_HERE, [this, vendor_name] {
    if (providers_.find(vendor_name) == providers_.end()) {
      return -ERR_NOT_READY;
    }
    providers_.erase(vendor_name);
    return 0;
  });
}

agora_refptr<IVideoFrame> ExtensionControlImpl::createVideoFrame(IVideoFrame::Type type,
                                                                 IVideoFrame::Format format,
                                                                 int width, int height) {
#ifdef FEATURE_VIDEO
  if (type == IVideoFrame::Type::kTexture) {
    // TODO(Yaqi): implementation for texture
    return nullptr;
  }
  return ExternalVideoFrame::Create(format, width, height);
#else
  return nullptr;
#endif  // FEATURE_VIDEO
}

agora_refptr<IVideoFrame> ExtensionControlImpl::copyVideoFrame(agora_refptr<IVideoFrame> src) {
#ifdef FEATURE_VIDEO
  if (!src) {
    return nullptr;
  }
  auto result = createVideoFrame(src->type(), src->format(), src->width(), src->height());
  if (!result) {
    return nullptr;
  }
  result->setRotation(src->rotation());
  result->setTimeStampUs(src->timestampUs());
  memcpy(result->mutableData(), src->data(), src->size());
  return result;
#else
  return nullptr;
#endif  // FEATURE_VIDEO
}

void ExtensionControlImpl::recycleVideoCache(IVideoFrame::Type type) {
#ifdef FEATURE_VIDEO
  if (type == IVideoFrame::Type::kTexture) {
    // TODO(Yaqi): implementation
    return;
  }
  webrtc::GlobalVideoFrameBufferPool::Instance().ReleaseExternalRawDataCacheMemory();
#endif  // FEATURE_VIDEO
}

int ExtensionControlImpl::dumpVideoFrame(agora_refptr<IVideoFrame> frame, const char* file) {
#ifdef FEATURE_VIDEO
  if (file == nullptr || !frame) {
    return -ERR_INVALID_ARGUMENT;
  }
  // TODO(Yaqi): Figure out right way of dumping external video frames. The following implementation
  // is just a prototype.
  std::lock_guard<std::mutex> _(dump_lock_);
  std::fstream ofile = std::fstream(file, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!ofile.is_open()) {
    return -ERR_INVALID_ARGUMENT;
  }
  ofile.write(reinterpret_cast<const char*>(frame->data()), frame->size());
  return ERR_OK;
#else
  return -ERR_NOT_SUPPORTED;
#endif  // FEATURE_VIDEO
}

int ExtensionControlImpl::log(commons::LOG_LEVEL level, const char* message) {
  commons::log(commons::AgoraLogger::ApiLevelToUtilLevel(level), "%s: %s", MODULE_NAME, message);
  return ERR_OK;
}

}  // namespace rtc
}  // namespace agora
