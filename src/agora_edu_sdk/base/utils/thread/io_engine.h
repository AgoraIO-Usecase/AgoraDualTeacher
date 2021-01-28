//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "utils/thread/io_engine_base.h"

#if defined(FEATURE_EVENT_ENGINE)
#include "utils/net/event_http_client.h"
#include "utils/net/event_http_client2.h"
#include "utils/net/event_tcp_client.h"
#include "utils/net/event_udp_server.h"
#include "utils/thread/internal/event_async_queue.h"
#include "utils/thread/internal/event_engine.h"
#endif  // FEATURE_EVENT_ENGINE

#include "utils/net/dns_parser.h"

namespace agora {
	namespace commons {

#if defined(FEATURE_EVENT_ENGINE)
		namespace libevent {
			dns_parser_base* create_dns_parser(dns_parser_manager* mgr, dns_id_t id);
		}  // namespace libevent
#endif  // FEATURE_EVENT_ENGINE

		class io_engine_factory {
			typedef std::function<io_engine_base*(void)> create_io_engine_fn;
			typedef std::function<udp_server_base*(io_engine_base* engine, udp_server_callbacks&& callbacks)>
				create_udp_server_fn;
			typedef std::function<tcp_client_base*(io_engine_base* engine, const ip::sockaddr_t& addr,
				tcp_client_callbacks&& callbacks, bool keep_alive)>
				create_tcp_client_fn;
			typedef std::function<http_client_base*(io_engine_base* engine, const std::string& url,
				http_client_callbacks&& callbacks,
				const std::string& hostname)>
				create_http_client_fn;
			typedef std::function<http_client_base2*(
				io_engine_base* engine, const std::string& url, http_client2_callbacks&& callbacks,
				const std::string& hostname, uint16_t port, bool security)>
				create_http_client2_fn;
			typedef dns_parser_manager::dns_parser_creator create_dns_parser_fn;

		public:
			io_engine_factory()
				: create_io_engine_(nullptr),
				create_dns_parser_(nullptr),
				create_udp_server_(nullptr),
				create_tcp_client_(nullptr),
				create_http_client_(nullptr),
				create_http_client2_(nullptr) {
#if defined(FEATURE_EVENT_ENGINE)
				create_io_engine_ = [] { return new libevent::event_engine(false); };
				create_dns_parser_ = &agora::commons::libevent::create_dns_parser;

				create_udp_server_ = [](io_engine_base* engine, udp_server_callbacks&& callbacks) {
					return new libevent::udp_server(*static_cast<libevent::event_engine*>(engine),
						std::move(callbacks));
				};

#if defined(FEATURE_TCP)
				create_tcp_client_ = [](io_engine_base* engine, const ip::sockaddr_t& addr,
					tcp_client_callbacks&& callbacks, bool keep_alive) {
						return new libevent::tcp_client(*static_cast<libevent::event_engine*>(engine), addr,
							std::move(callbacks), keep_alive);
				};
#endif  // FEATURE_TCP

#if defined(FEATURE_HTTP)
				create_http_client_ = [](io_engine_base* engine, const std::string& url,
					http_client_callbacks&& callbacks, const std::string& hostname) {
						return new libevent::http_client(*static_cast<libevent::event_engine*>(engine), url,
							std::move(callbacks), hostname);
				};

				create_http_client2_ = [](io_engine_base* engine, const std::string& url,
					agora::commons::http_client2_callbacks&& callbacks,
					const std::string& hostname, uint16_t port, bool security) {
						return new libevent::http_client2(*static_cast<libevent::event_engine*>(engine), url,
							std::move(callbacks), hostname, port, security);
				};
#endif  // FEATURE_HTTP
#endif  // FEATURE_EVENT_ENGINE
			}

			io_engine_base* create_io_engine() { return create_io_engine_ ? create_io_engine_() : nullptr; }

			template <typename T>
			async_queue_base<T>* create_async_queue(io_engine_base* engine,
				typename async_queue_base<T>::callback_type&& cb) {
#if defined(FEATURE_EVENT_ENGINE)
				return libevent::create_async_queue<T>(engine, std::move(cb));
#endif  // FEATURE_EVENT_ENGINE
				return nullptr;
			}

			template <typename T1, typename T2>
			async_queue_base<T1, T2>* create_promise_async_queue(
				io_engine_base* engine, typename async_queue_base<T1, T2>::callback_type&& cb,
				const std::string& thread_name) {
#if defined(FEATURE_EVENT_ENGINE)
				return libevent::create_promise_async_queue<T1, T2>(engine, std::move(cb), thread_name);
#endif  // FEATURE_EVENT_ENGINE
				return nullptr;
			}

			create_dns_parser_fn get_dns_parser_creator() const { return create_dns_parser_; }

			udp_server_base* create_udp_server(
				io_engine_base* engine,
				udp_server_callbacks&& callbacks = agora::commons::udp_server_callbacks()) {
				return create_udp_server_ ? create_udp_server_(engine, std::move(callbacks)) : nullptr;
			}

			tcp_client_base* create_tcp_client(io_engine_base* engine, const ip::sockaddr_t& addr,
				tcp_client_callbacks&& callbacks, bool keep_alive = true) {
				return create_tcp_client_ ? create_tcp_client_(engine, addr, std::move(callbacks), keep_alive)
					: nullptr;
			}

			bool has_http_client() const { return create_http_client_ != nullptr; }
			http_client_base* create_http_client(io_engine_base* engine, const std::string& url,
				agora::commons::http_client_callbacks&& callbacks,
				const std::string& hostname = "") {
				return create_http_client_ ? create_http_client_(engine, url, std::move(callbacks), hostname)
					: nullptr;
			}

			bool has_http_client2() const { return create_http_client2_ != nullptr; }
			http_client_base2* create_http_client2(io_engine_base* engine, const std::string& url,
				agora::commons::http_client2_callbacks&& callbacks,
				const std::string& hostname, uint16_t port = 80,
				bool security = false) {
				return create_http_client2_
					? create_http_client2_(engine, url, std::move(callbacks), hostname, port, security)
					: nullptr;
			}

			bool has_ping_client() const {
				// hanging up a phone call on iPhone via power button and return to voice call
				// will trigger a sigpipe signal which crash the app  so disable ping on iOS
				// windows icmp library has bug which may causes heap corruption when
				// enable/disable network connection
#if !defined(_WIN32)
				return true;
#else
				return false;
#endif  // !_WIN32
			}

		private:
			create_io_engine_fn create_io_engine_;
			create_dns_parser_fn create_dns_parser_;
			create_udp_server_fn create_udp_server_;
			create_tcp_client_fn create_tcp_client_;
			create_http_client_fn create_http_client_;
			create_http_client2_fn create_http_client2_;
		};

	}  // namespace commons
}  // namespace agora
