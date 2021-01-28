//
//  Agora Media SDK
//
//  Modified by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <list>
#include <memory>

#include "utils/net/ping_client.h"
#include "utils/thread/internal/event_engine.h"
#include "utils/thread/io_engine_base.h"
#include "utils/tools/util.h"

namespace agora {
namespace commons {
class io_engine_base;
class io_engine;

/**
 * @brief The ping_client class
 * @note ping client.
 */
class ping_client_libevent : private noncopyable {
  using sink_type = ping_client::sink_type;
  typedef uint64_t counter_t;

  /* How to keep track of each host to ping */
  struct evhost {
    ip_t dest_ip; /* Internet address                        */
    uint8_t seq;  /* ICMP sequence (modulo 256) for next run */

    std::unique_ptr<timer_base> noreply_timer; /* Timer to handle ICMP timeout            */
    std::unique_ptr<timer_base> ping_timer;    /* Timer to ping host at given intervals   */

    /* Packets Counters */
    counter_t sentpkts; /* Total # of ICMP Echo Requests sent      */
    counter_t recvpkts; /* Total # of ICMP Echo Replies received   */
    counter_t dropped;  /* # of ICMP packets dropped               */

    /* Bytes counters */
    counter_t sentbytes; /* Total # of bytes sent                   */
    counter_t recvbytes; /* Total # of bytes received               */

    /* Timestamps */
    uint64_t firstsent; /* Time first ICMP request was sent        */
    uint64_t firstrecv; /* Time first ICMP reply was received      */
    uint64_t lastsent;  /* Time last ICMP request was sent         */
    uint64_t lastrecv;  /* Time last ICMP reply was received       */

    /* Counters for statistics */
    uint32_t shortest; /* Shortest reply time                     */
    uint32_t longest;  /* Longest reply time                      */
    uint32_t sum;      /* Sum of reply times                      */
    uint32_t square;   /* Sum of qquare of reply times            */

    evhost();
    evhost(evhost&& rhs);
    evhost(const evhost&) = delete;
    void clear();
  };
  /* How to keep track of a PING session */
  struct evping {
    uint32_t pktsize; /* Packet size in bytes (ICMP plus User Data) */

    int noreply_interval; /* ICMP Echo Reply timeout                    */
    int ping_interval;    /* Ping interval between two subsequent pings */

    counter_t sendfail; /* # of failed sendto()                       */
    counter_t sentok;   /* # of successful sendto()                   */
    counter_t recvfail; /* # of failed recvfrom()                     */
    counter_t recvok;   /* # of successful recvfrom()                 */
    counter_t tooshort; /* # of ICMP packets too short (illegal ICMP) */
    counter_t foreign;  /* # of ICMP packets we are not looking for   */
    counter_t illegal;  /* # of ICMP packets with an illegal payload  */

    uint16_t ident; /* Identifier to send with each ICMP Request  */
    uint8_t quiet;
  };

 public:
  enum PING_ERROR_TYPE {
    PING_ERR_NONE = 0,
    PING_ERR_TIMEOUT = 1,   /* Communication with the host timed out */
    PING_ERR_SHUTDOWN = 10, /* The request was canceled because the PING subsystem was shut down */
    PING_ERR_CANCEL = 12,   /* The request was canceled via a call to evping_cancel_request */
    PING_ERR_UNKNOWN = 16,  /* An unknown error occurred */
  };
  explicit ping_client_libevent(io_engine_factory* factory, io_engine_base* engine,
                                sink_type&& sink, int timeout = 2000);
  ~ping_client_libevent();
  int add_address(const ip_t& dest_ip, int interval, const ip_t* local_address);
  int remove_address(const ip_t& dest_ip);
  int remove_all_addresses();
  void ping(sink_type&& callback);
  void stats();
  size_t size() const { return hosts_.size(); }
  const char* error_to_string(int err);

 private:
  int initialize();
  static void ping_data_callback(evutil_socket_t fd, int16_t event, void* context);
  void on_data(evutil_socket_t fd, int16_t event);
  void on_ping_timer(evhost* host);
  void on_noreply(evhost* host);
  evhost* find_host(const ip_t& dest_ip);

 private:
  libevent::event_engine& net_;
  sink_type sink_;
  event* e_;
  evping data_;
  std::list<evhost> hosts_;
  int timeout_;
  bool using_ping_socket_;
};
}  // namespace commons
}  // namespace agora
