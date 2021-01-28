//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-04.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

#include "api2/NGIAgoraExtensionControl.h"

namespace agora {

namespace base {
class AgoraService;
}

namespace rtc {

class ExtensionControlImpl : public IExtensionControl {
 public:
  ~ExtensionControlImpl() = default;
  ExtensionControlImpl(const ExtensionControlImpl&) = delete;
  ExtensionControlImpl& operator=(const ExtensionControlImpl&) = delete;
  ExtensionControlImpl(ExtensionControlImpl&&) = delete;
  ExtensionControlImpl& operator=(ExtensionControlImpl&&) = delete;

  static ExtensionControlImpl* GetInstance();

  agora_refptr<IExtensionProvider> getExtensionProvider(const char* vendor_name);

  // inherited from IExtensionControl
  void getCapabilities(Capabilities& capabilities) override;

  int registerExtensionProvider(
      const char* vendor_name,
      agora::agora_refptr<agora::rtc::IExtensionProvider> provider) override;

  int unregisterExtensionProvider(const char* vendor_name) override;

  agora_refptr<IVideoFrame> createVideoFrame(IVideoFrame::Type type, IVideoFrame::Format format,
                                             int width, int height) override;

  agora_refptr<IVideoFrame> copyVideoFrame(agora_refptr<IVideoFrame> src) override;

  void recycleVideoCache(IVideoFrame::Type type) override;

  int dumpVideoFrame(agora_refptr<IVideoFrame> frame, const char* file) override;

  int log(commons::LOG_LEVEL level, const char* message) override;

 private:
  friend class agora::base::AgoraService;

  static ExtensionControlImpl* Create();
  static void Destroy(ExtensionControlImpl* p);
  ExtensionControlImpl() = default;

 private:
  using ProvidersMap = std::unordered_map<std::string, agora::agora_refptr<IExtensionProvider>>;
  ProvidersMap providers_;
  std::mutex dump_lock_;
};

}  // namespace rtc
}  // namespace agora
