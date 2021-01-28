//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/transport/tls_handler.h"

#include "openssl/err.h"
#include "utils/log/log.h"

namespace agora {
namespace transport {
using agora::commons::log;
using agora::commons::LOG_WARN;

TlsHandler::~TlsHandler() { Free(); }

bool TlsHandler::Initialize(const std::string& verify_domain) {
#ifdef RTC_BUILD_SSL
  Free();
  ssl_ = SSL_new(ssl_context_);
  if (!ssl_) {
    return false;
  }
  receiver_bio_ = BIO_new(BIO_s_mem());
  if (!receiver_bio_) {
    Free();
    return false;
  }
  writer_bio_ = BIO_new(BIO_s_mem());
  if (!writer_bio_) {
    BIO_free(receiver_bio_);
    Free();
    return false;
  }
  SSL_set_bio(ssl_, receiver_bio_, writer_bio_);
  SSL_set_connect_state(ssl_);
  verify_domain_ = verify_domain;
  return true;
#else
  return false;
#endif
}

bool TlsHandler::Connect() {
#ifdef RTC_BUILD_SSL
  if (!ssl_) {
    return false;
  }
  if (!IsDisconnected()) {
    return false;
  }
  if (!verify_domain_.empty()) {
    auto param = SSL_get0_param(ssl_);
    X509_VERIFY_PARAM_set1_host(param, verify_domain_.data(), verify_domain_.size());
  }
  state_ = State::kConnecting;
  SSL_do_handshake(ssl_);
  WriteEncryptData();
  return true;
#else
  return false;
#endif
}

int TlsHandler::ReceiveData(const char* data, std::size_t length) {
#ifdef RTC_BUILD_SSL
  if (IsDisconnected()) {
    return 0;
  }
  int total_write = 0;
  while (length > 0) {
    auto written = BIO_write(receiver_bio_, data, length);
    if (written <= 0) {
      log(LOG_WARN, "[bio] failed to write received data");
      // TODO(zhangzhejun) notify the connnection is broken?
      return 0;
    }
    length -= written;
    data += written;
    total_write += written;
    if (IsConnecting()) {
      auto handshake_result = SSL_do_handshake(ssl_);
      auto err = ERR_get_error();
      if (handshake_result == 1) {
        state_ = State::kConnected;
        observer_->OnTlsConnect(true);
      } else if (err == 0) {
        WriteEncryptData();
      } else {
        char buf[256];
        ERR_error_string_n(err, buf, 256);
        log(LOG_WARN, "[tls] failed handshake with code: %u, %s", err, buf);
        Broken();
      }
    } else {
      EnsureBufferAvailable();
      int read = 0;
      do {
        read = SSL_read(ssl_, buffer_.data(), buffer_.size());
        if (read > 0) {
          observer_->OnDecryptedData(buffer_.data(), read);
        } else {
          break;
        }
      } while (true);
    }
  }
  return total_write;
#else
  return 0;
#endif
}

int TlsHandler::SendData(const char* data, std::size_t length) {
#ifdef RTC_BUILD_SSL
  if (!IsConnected()) {
    return 0;
  }
  int total_send_bytes = 0;
  while (length > 0) {
    auto written = SSL_write(ssl_, data, length);
    if (!WriteEncryptData()) {
      return -1;
    }
    length -= written;
    data += written;
    total_send_bytes += written;
  }
  return total_send_bytes;
#else
  return 0;
#endif
}

TlsHandler::TlsHandler(ITlsHandlerObserver* observer, SSL_CTX* context)
    : observer_(observer),
      ssl_context_(context),
      ssl_(nullptr),
      receiver_bio_(nullptr),
      writer_bio_(nullptr),
      state_(State::kDisconnected) {}

void TlsHandler::Free() {
#ifdef RTC_BUILD_SSL
  if (!ssl_context_) {
    return;
  }
  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
    receiver_bio_ = nullptr;
    writer_bio_ = nullptr;
  }
#endif
  state_ = State::kDisconnected;
}

bool TlsHandler::WriteEncryptData() {
#ifdef RTC_BUILD_SSL
  if (IsDisconnected() || IsBroken()) {
    return false;
  }
  while (auto pending_bytes = BIO_ctrl_pending(writer_bio_)) {
    // FIXME: zhangzhejun allocate the fixed buffer size and handle the EAGAIN.
    EnsureBufferAvailable(pending_bytes);
    auto read = BIO_read(writer_bio_, buffer_.data(), buffer_.size());
    if (read > 0) {
      auto result = observer_->OnEncryptedData(buffer_.data(), read);
      if (result <= 0) {
        return false;
      }
    }
  }
#endif
  return true;
}

void TlsHandler::EnsureBufferAvailable(std::size_t bytes) {
  if (buffer_.size() > bytes) {
    return;
  }
  std::size_t size = bytes == 0 ? kEncryptionBufferSize : bytes;
  buffer_.resize(size);
}

void TlsHandler::Broken() {
  state_ = State::kBroken;
  observer_->OnTlsConnect(false);
}

}  // namespace transport
}  // namespace agora
