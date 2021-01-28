//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <cinttypes>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <thread>


#include "utils/log/log.h"
#include "utils/net/ip_type.h"
#include "utils/packer/packer.h"
#include "utils/tools/util.h"

#define USE_VIRTUAL_METHOD

#if defined(USE_VIRTUAL_METHOD)
#define OVERRIDE override
#else
#define OVERRIDE
#endif  // USE_VIRTUAL_METHOD

namespace agora {
namespace commons {

struct packet;
class port_allocator;
class socks5_client;
struct perf_counter_data;

class timer_base {
 public:
  using callback_type = std::function<void(timer_base* thiz)>;
  virtual ~timer_base() {}
  virtual void schedule(uint64_t ms) = 0;
  virtual void cancel() = 0;
};

class io_engine_base {
 public:
  virtual ~io_engine_base() {}
  virtual void run() = 0;
  virtual void run_nonblock() = 0;
  virtual bool is_valid() const = 0;
  virtual void break_loop() = 0;
  virtual void set_priorities(int prio) {}
  virtual timer_base* create_timer(timer_base::callback_type&& f, uint64_t ms,
                                   bool persist = true) = 0;
  virtual void reset_bytes() = 0;
  virtual void inc_tx_bytes(size_t length) = 0;
  virtual void inc_rx_bytes(size_t length) = 0;
  virtual void inc_damaged_packets() = 0;
  virtual void inc_exceed_mtu_packets() = 0;
  virtual size_t tx_bytes() const = 0;
  virtual size_t rx_bytes() const = 0;
  virtual uint16_t last_rx_bytes() const = 0;
  virtual size_t tx_packets() const = 0;
  virtual size_t rx_packets() const = 0;
  virtual size_t damaged_packets() const = 0;
  virtual size_t exceed_mtu_packets() const = 0;
};

template <typename T1, typename T2 = int>
class async_queue_base {
 public:
  using callback_type = std::function<void(const T1&)>;

 public:
  struct async_queue_task {
   public:
    async_queue_task() = default;

    explicit async_queue_task(T1&& normal_task)
        : task(std::move(normal_task)) {}

    async_queue_task(T1&& normal_task, const std::string& loc)
        : task(std::move(normal_task)), location(loc) {}

    async_queue_task(T1&& normal_task, uint64_t task_id,
                     const std::set<std::thread::id>& invoker_thrds,
                     const std::string& loc)
        : task(std::move(normal_task)),
          id(task_id),
          invoker_threads(invoker_thrds),
          location(loc) {}

    async_queue_task(async_queue_task&& rhs)
        : task(std::move(rhs.task)),
          id(rhs.id),
          parent_thrd_id(rhs.parent_thrd_id),
          invoker_threads(std::move(rhs.invoker_threads)),
          location(std::move(rhs.location)),
          executed(rhs.executed) {
      // for std::function on Linux, sometimes it won't be set to 'Empty' in its
      // move ctor/assignment, e.g. when only capture some basic types like int
      rhs.task = nullptr;
      rhs.id = 0;
      rhs.parent_thrd_id = std::thread::id();
      rhs.executed = false;
    }

    ~async_queue_task() {
      if (task && !executed && !location.empty()) {
        // log(LOG_WARN,
        //    "async_queue_task: destroying UNEXECUTED task: id: %" PRId64
        //    ", location: %s",
        //    id, location.c_str());
      }
    }

    async_queue_task& operator=(async_queue_task&& rhs) {
      if (&rhs == this) return *this;

      task = std::move(rhs.task);
      // for std::function on Linux, sometimes it won't be set to 'Empty' in its
      // move ctor/assignment, e.g. when only capture some basic types like int
      rhs.task = nullptr;

      id = rhs.id;
      rhs.id = 0;

      parent_thrd_id = rhs.parent_thrd_id;
      rhs.parent_thrd_id = std::thread::id();

      invoker_threads = std::move(rhs.invoker_threads);
      location = std::move(rhs.location);

      rhs.executed = false;

      return *this;
    }

    // no copy constructor or copy assignment because we don't want two lambda
    // hold same shared pointer
    async_queue_task(const async_queue_task& rhs) = delete;
    async_queue_task& operator=(const async_queue_task& rhs) = delete;

    T1 task;
    uint64_t id = 0;
    // unused
    std::thread::id parent_thrd_id = std::this_thread::get_id();
    std::set<std::thread::id> invoker_threads;
    std::string location;
    mutable volatile bool executed = false;
  };

 public:
  virtual ~async_queue_base() = default;

  virtual int async_call(T1&& normal_task, uint64_t ts = 0) = 0;
  virtual int async_call(async_queue_task&& queue_task, uint64_t ts = 0) = 0;
  virtual int await_async_call(T1&& req, T2* res, int timeout = 0) {
    return -1;
  }

  virtual void set_priority(int prio) = 0;
  virtual void set_capacity(size_t capacity) = 0;

  virtual size_t size() const = 0;
  virtual bool empty() const = 0;

  virtual bool poll_tasks(std::thread::id invoker_thread) = 0;

  virtual void clear(bool do_remain = true) = 0;
  virtual void close() = 0;
  virtual bool closed() const = 0;

  virtual bool get_perf_counter_data(perf_counter_data& data) = 0;
  virtual uint64_t last_pop_ts() const = 0;

  virtual int wait_empty(int64_t wait_ms) { return 0; }

  virtual void set_promise_result(T2&& result) {}

 protected:
  async_queue_base() = default;
};

class udp_server_base;
struct default_udp_packet_handler {
	typedef std::function<void(udp_server_base*, const ip::sockaddr_t&, unpacker&, uint16_t,
		uint16_t)>
		packet_cb_type;
	packet_cb_type on_packet;
	explicit default_udp_packet_handler(packet_cb_type&& packet_cb) : on_packet(packet_cb) {}
	bool operator()(udp_server_base* s, const ip::sockaddr_t& peer_addr, const char* data,
		size_t length) {
		unpacker p(data, length);
		uint16_t packet_length = p.pop_uint16();
		if (packet_length > length) {
			log(LOG_WARN, "damaged udp packet from %s, packet length %u exceeded data length %u!",
				commons::desensetizeIp(ip::to_string(peer_addr)).c_str(), packet_length, length);
			return false;
		}

		uint16_t server_type = -1;
		uint16_t uri = -1;
		//    try {
		server_type = p.pop_uint16();
		uri = p.pop_uint16();
		p.rewind();
		on_packet(s, peer_addr, p, server_type, uri);
		return true;
	}
};

struct udp_server_callbacks {
	typedef std::function<bool(udp_server_base*, const ip::sockaddr_t&, const char* data,
		size_t length)>
		data_cb_type;
	typedef std::function<void(udp_server_base*, int err)> error_cb_type;
	data_cb_type on_data;
	error_cb_type on_error;
	explicit udp_server_callbacks(data_cb_type&& data_cb = nullptr,
		error_cb_type&& error_cb = nullptr)
		: on_data(std::move(data_cb)), on_error(std::move(error_cb)) {}
};

class udp_server_base {
public:
	virtual ~udp_server_base() {}
	virtual void close() = 0;
	virtual void set_port_allocator(std::shared_ptr<port_allocator>& p) = 0;
	virtual void set_proxy_server(const socks5_client* proxy) = 0;
	virtual void set_callbacks(udp_server_callbacks&& callbacks) = 0;
	virtual bool bind(int af_family, const ip_t& ip = ip_t(), uint16_t port = 0,
		size_t tries = 1) = 0;
	virtual int send_message(const ip::sockaddr_t& peer_addr, const packet& p) = 0;
	virtual int send_message(const ip::sockaddr_t& peer_addr, const packet& p,
		size_t& sent_length) = 0;
	virtual int send_buffer(const ip::sockaddr_t& peer_addr, const char* data, size_t length) = 0;
	virtual const ip::sockaddr_t& local_address() const = 0;
	virtual bool binded() const = 0;
	virtual int set_socket_buffer_size(int bufsize) = 0;
	virtual void set_max_buffer_size(size_t length) = 0;
	virtual int get_socket_fd() const = 0;
};

class tcp_client_base;
struct default_tcp_packet_handler {
	typedef std::function<void(tcp_client_base* client, unpacker& p, uint16_t server_type,
		uint16_t uri)>
		packet_cb_type;
	packet_cb_type on_packet;
	explicit default_tcp_packet_handler(packet_cb_type&& packet_cb) : on_packet(packet_cb) {}
	int operator()(tcp_client_base* client, const char* data, size_t length) {
		if (length <= 2) return 0;
		unpacker p(data, length);
		uint16_t packet_length = p.pop_uint16();
		if (packet_length > length) return 0;  // packet is incomplete
		uint16_t server_type = p.pop_uint16();
		uint16_t uri = p.pop_uint16();
		p.rewind();

#if AGORARTC_HAS_EXCEPTION
		try {
			on_packet(client, p, server_type, uri);
		}
		catch (std::overflow_error& e) {
			log(LOG_WARN, "error on tcp packet %u from %s %s", uri, remote_addr().c_str(), e.what());
		}
#else
		on_packet(client, p, server_type, uri);
#endif
		return packet_length;
	}
};

struct tcp_client_callbacks {
	typedef std::function<void(tcp_client_base* client, bool connected)> connect_cb_type;
	typedef std::function<int(tcp_client_base* client, const char* data, size_t length)> data_cb_type;
	typedef std::function<void(tcp_client_base* client)> event_cb_type;
	connect_cb_type on_connect;
	data_cb_type on_data;
	event_cb_type on_socket_error;
	event_cb_type on_ping_cycle;
	tcp_client_callbacks(connect_cb_type&& connect_cb = nullptr, data_cb_type&& data_cb = nullptr,
		event_cb_type&& socket_error_cb = nullptr,
		event_cb_type&& ping_cycle_cb = nullptr)
		: on_connect(std::move(connect_cb)),
		on_data(std::move(data_cb)),
		on_socket_error(std::move(socket_error_cb)),
		on_ping_cycle(std::move(ping_cycle_cb)) {}
};

class tcp_client_base {
public:
	static void delete_later(std::unique_ptr<tcp_client_base>& p) {
		if (p) {
			p->close(true);
			p.release();
		}
	}
	virtual ~tcp_client_base() {}
	virtual void set_timeout(uint32_t timeout) = 0;
	virtual bool is_connected() const = 0;
	virtual bool is_closed() const = 0;
	virtual bool connect() = 0;
	virtual void close(bool delete_later) = 0;
	virtual void set_proxy_server(const socks5_client* proxy) = 0;
	virtual void set_callbacks(tcp_client_callbacks&& callbacks) = 0;
	virtual int send_message(const packet& p) = 0;
	virtual int send_buffer(const char* data, uint32_t length) = 0;
	virtual int flush_out(const packet& p) { return 0; }
	virtual int flush_out(const char* data, uint32_t length) { return 0; }
	virtual std::string remote_addr() const = 0;
	virtual const ip::sockaddr_t& remote_socket_address() const = 0;
	virtual ip_t remote_ip() const = 0;
	virtual uint16_t remote_port() const = 0;
};
struct http_client_callbacks {
	typedef std::function<void(int err)> event_type;
	event_type on_request;
	event_type on_request_chunked;
	http_client_callbacks(event_type&& request_cb = nullptr,
		event_type&& request_chunked_cb = nullptr)
		: on_request(std::move(request_cb)), on_request_chunked(std::move(request_chunked_cb)) {}
};

class http_client_base {
public:
	virtual ~http_client_base() {}
	virtual int initialize() = 0;
	virtual void set_callbacks(http_client_callbacks&& callbacks) = 0;
	virtual int get_data(char* buffer, size_t length) = 0;
};

struct http_client2_callbacks {
	enum http_client_event {
		http_client_event_send_complete,
		http_client_event_response_received,
		http_client_event_error
	};

	//add for special handler
	static const int ROLE_FULL_STATUS = 600;

	typedef std::function<void(enum http_client_event ev, int err)> event_type;
	typedef std::function<void(const char* buf, size_t length)> data_type;
	event_type on_request;
	data_type on_data;
	explicit http_client2_callbacks(event_type&& request_cb = nullptr)
		: on_request(std::move(request_cb)) {}
};

/* class http_client_base2 is mainly used to post information to remote server
 */

constexpr int HTTP_STATUS_OK = 200;

enum HTTP_METHOD { HTTP_METHOD_GET, HTTP_METHOD_PUT, HTTP_METHOD_POST, HTTP_METHOD_DELETE };

class http_client_base2 {
public:
	virtual ~http_client_base2() {}
	virtual int http_add_header(const std::string& key, const std::string& value) = 0;
	virtual int http_add_body_buff(const std::string& body) = 0;
	virtual void set_callbacks(http_client2_callbacks&& callbacks) = 0;
	virtual int make_request(HTTP_METHOD method) = 0;
};

class io_engine_factory_base {
	typedef udp_server_base* (*create_udp_server_fn)(io_engine_base* engine,
		udp_server_callbacks&& callbacks);
	typedef tcp_client_base* (*create_tcp_client_fn)(io_engine_base* engine,
		const ip::sockaddr_t& addr,
		tcp_client_callbacks&& callbacks,
		bool keep_alive, bool read_header_length);
	typedef http_client_base* (*create_http_client_fn)(io_engine_base* engine, const std::string& url,
		const std::string& hostname,
		http_client_callbacks&& callbacks);
	typedef http_client_base2* (*create_http_client2_fn)(io_engine_base* engine,
		const std::string& url,
		const std::string& hostname,
		http_client2_callbacks&& callbacks,
		uint16_t port);

public:
	create_udp_server_fn create_udp_server_;
	create_tcp_client_fn create_tcp_client_;
	create_http_client_fn create_http_client_;
	create_http_client2_fn create_http_client2_;
};

}  // namespace commons
}  // namespace agora
