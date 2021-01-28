//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <string>
#include <vector>

#include "openssl/ssl.h"

namespace agora {
namespace transport {
namespace testing {
class TestTlsHandler;
}

class ITlsHandlerObserver {
 public:
  virtual ~ITlsHandlerObserver() {}
  virtual void OnTlsConnect(bool success) = 0;
  virtual void OnDecryptedData(const char* data, std::size_t length) = 0;
  virtual int OnEncryptedData(const char* data, std::size_t length) = 0;
};

class ITlsHandler {
 public:
  virtual bool Initialize(const std::string&) = 0;
  virtual bool Connect() = 0;
  virtual int ReceiveData(const char*, std::size_t) = 0;
  virtual int SendData(const char*, std::size_t) = 0;
  virtual bool IsConnected() const = 0;
  virtual bool IsConnecting() const = 0;
  virtual bool IsDisconnected() const = 0;
  virtual bool IsBroken() const = 0;
  virtual ~ITlsHandler() {}
};

class TlsHandler : public ITlsHandler {
  friend class TlsManager;
  friend class testing::TestTlsHandler;

 public:
  ~TlsHandler();
  bool Initialize(const std::string& verify_domain = "");
  bool Connect();
  int ReceiveData(const char* data, std::size_t length);
  int SendData(const char* data, std::size_t length);
  bool IsConnected() const;
  bool IsConnecting() const;
  bool IsDisconnected() const;
  bool IsBroken() const;

 private:
  TlsHandler(ITlsHandlerObserver* observer, SSL_CTX* context);
  void Free();
  bool WriteEncryptData();
  void EnsureBufferAvailable(std::size_t bytes = 0);
  void Broken();

  enum class State {
    kDisconnected,
    kConnecting,
    kConnected,
    kBroken,
  };
  enum { kEncryptionBufferSize = 4096 };
  ITlsHandlerObserver* observer_;
  SSL_CTX* ssl_context_;
  SSL* ssl_;
  BIO* receiver_bio_;
  BIO* writer_bio_;
  State state_;
  std::vector<char> buffer_;
  std::string verify_domain_;
};

inline bool TlsHandler::IsConnected() const { return state_ == State::kConnected; }

inline bool TlsHandler::IsConnecting() const { return state_ == State::kConnecting; }

inline bool TlsHandler::IsDisconnected() const { return state_ == State::kDisconnected; }

inline bool TlsHandler::IsBroken() const { return state_ == State::kBroken; }

}  // namespace transport
}  // namespace agora
