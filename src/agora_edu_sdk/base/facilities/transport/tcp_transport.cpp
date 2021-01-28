//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "facilities/transport/tcp_transport.h"

#include "facilities/transport/tls_manager.h"

namespace agora {
namespace transport {

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

class DestroyGuard {
 public:
  explicit DestroyGuard(TcpTransport* transport, bool late_delete_channel = false)
      : transport_(transport),
        late_delete_channel_(late_delete_channel),
        already_in_processing_(transport_->processing_flag_) {
    if (!already_in_processing_) {
      transport_->processing_flag_ = true;
    }
  }
  ~DestroyGuard() {
    if (already_in_processing_) {
      return;
    }
    transport_->processing_flag_ = false;
    if (transport_->late_destroy_) {
      if (late_delete_channel_) {
        commons::tcp_client_base::delete_later(transport_->channel_);
      }
      delete transport_;
    }
  }

 private:
  TcpTransport* transport_;
  bool late_delete_channel_;
  bool already_in_processing_;
};

TcpTransport::TcpTransport(INetworkTransportObserver* observer, ITlsManager* tls_manager,
                           const std::string& domain, agora::base::BaseWorker* worker)
    : observer_(observer),
      tls_manager_(tls_manager),
      worker_(worker),
      verify_domain_(domain),
      processing_flag_(false),
      late_destroy_(false) {}

TcpTransport::~TcpTransport() {}

bool TcpTransport::Connect(const commons::ip::sockaddr_t& address) {
  if (channel_) {
    return false;
  }
  DestroyGuard _(this);
  remote_address_ = address;
  agora::commons::tcp_client_callbacks callbacks;
  callbacks.on_connect = std::bind(&TcpTransport::OnTcpConnect, this, _1, _2);
  callbacks.on_data = std::bind(&TcpTransport::OnTcpData, this, _1, _2, _3);
  callbacks.on_socket_error = std::bind(&TcpTransport::OnTcpSocketError, this, _1);
  channel_.reset(worker_->createTcpClient(address, std::move(callbacks)));
  channel_->connect();
  return true;
}

bool TcpTransport::Connect(const commons::ip::sockaddr_t& address,
                           const std::vector<uint8_t>& early_data) {
  return false;
}

int TcpTransport::SendMessage(const commons::packet& p) {
  commons::packer pk;
  p.pack(pk);
  return SendBuffer(pk.buffer(), pk.length());
}

int TcpTransport::SendBuffer(const char* data, std::size_t length) {
  if (!IsConnected()) {
    return -EFAULT;
  }
  if (tls_handler_) {
    auto r = tls_handler_->SendData(data, length);
    if (r >= 0 && static_cast<std::size_t>(r) == length) {
      return 0;
    } else {
      return -EFAULT;
    }
  }
  return channel_->send_buffer(data, length);
}

void TcpTransport::SetTimeout(uint32_t timeout) {
  if (channel_) {
    channel_->set_timeout(timeout);
  }
}

bool TcpTransport::IsConnected() const {
  if (!channel_) {
    return false;
  }
  if (!channel_->is_connected()) {
    return false;
  }
  if (tls_handler_) {
    return tls_handler_->IsConnected();
  }
  return true;
}

const commons::ip::sockaddr_t& TcpTransport::RemoteAddress() const { return remote_address_; }

TransportType TcpTransport::Type() const { return TransportType::kTcp; }

int TcpTransport::SocketFd() const { return -1; }

void TcpTransport::Destroy() {
  if (processing_flag_) {
    late_destroy_ = true;
    observer_ = nullptr;
  } else {
    delete this;
  }
}

void TcpTransport::SetNetworkTransportObserver(INetworkTransportObserver* observer) {
  observer_ = observer;
}

void TcpTransport::OnTlsConnect(bool success) { OnConnect(success); }

void TcpTransport::OnDecryptedData(const char* data, std::size_t length) {
  stream_buffer_->receive(data, length);
}

int TcpTransport::OnEncryptedData(const char* data, std::size_t length) {
  if (!channel_) {
    return -EFAULT;
  }
  auto r = channel_->send_buffer(data, length);
  if (!r) {
    return length;
  }
  return -EFAULT;
}

void TcpTransport::OnTcpConnect(commons::tcp_client_base* client, bool connected) {
  DestroyGuard _(this);
  if (connected) {
    if (tls_manager_) {
      tls_handler_.reset(tls_manager_->CreateHandler(this));
      if (tls_handler_) {
        tls_handler_->Initialize(verify_domain_);
      }
    }
    if (tls_handler_) {
      tls_handler_->Connect();
    } else {
      OnConnect(true);
    }
  } else {
    deferred_disconnect_timer_.reset(worker_->createTimer([this]() { OnConnect(false); }, 0));
  }
}

int TcpTransport::OnTcpData(commons::tcp_client_base* client, const char* data,
                            std::size_t length) {
  DestroyGuard _(this, true);
  if (tls_handler_) {
    tls_handler_->ReceiveData(data, length);
  } else {
    stream_buffer_->receive(data, length);
  }
  return length;
}

void TcpTransport::OnTcpSocketError(commons::tcp_client_base* client) {
  if (observer_) {
    observer_->OnError(this, TransportErrorType::kSocketError);
  }
}

void TcpTransport::OnConnect(bool connected) {
  if (connected) {
    if (!stream_buffer_) {
      stream_buffer_.reset(
          new commons::stream_buffer([this](const char* data, std::size_t length) -> int {
            if (observer_) {
              return observer_->OnData(this, data, length);
            }
            return length;
          }));
    }
  }
  deferred_disconnect_timer_.reset();
  if (observer_) {
    observer_->OnConnect(this, connected);
  }
}

}  // namespace transport
}  // namespace agora
