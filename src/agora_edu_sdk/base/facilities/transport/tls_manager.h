//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include "openssl/ssl.h"

namespace agora {
namespace transport {
namespace testing {
class TestTlsManager;
}

class ITlsHandler;
class ITlsHandlerObserver;
class ITlsManager {
 public:
  virtual void Initialize() = 0;
  virtual ITlsHandler* CreateHandler(ITlsHandlerObserver*) = 0;
  virtual bool Initialized() const = 0;

 protected:
  virtual ~ITlsManager() {}
};

class TlsManager : public ITlsManager {
  friend class testing::TestTlsManager;

 public:
  TlsManager();
  ~TlsManager();

  void Initialize();
  ITlsHandler* CreateHandler(ITlsHandlerObserver* observer);
  bool Initialized() const;

 private:
#ifdef RTC_BUILD_SSL
  bool LoadCACertsFromSystem(SSL_CTX* context);
#endif

  SSL_CTX* ssl_context_;
};

inline bool TlsManager::Initialized() const { return ssl_context_; }

}  // namespace transport
}  // namespace agora
