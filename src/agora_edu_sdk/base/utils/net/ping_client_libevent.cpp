//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "ping_client_libevent.h"

#include <event2/util.h>
#include "utils/log/log.h"
#include "utils/tools/sys_type.h"
#include <algorithm>
#include <cerrno>
#include <cmath>
#if defined(__linux__)
#include <netinet/ip_icmp.h>
#endif

#ifndef ICMP_MINLEN
#define ICMP_MINLEN 8 /* abs minimum */
#endif

#ifndef IP_MAXPACKET
#define IP_MAXPACKET 65535
#endif

#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0
#endif

#if !defined(__linux__) || defined(__ANDROID__)
#if !defined(NR_ICMP_TYPES)
struct icmphdr {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  union {
    struct {
      uint16_t id;
      uint16_t sequence;
    } echo;
    uint32_t gateway;
    struct {
      uint16_t unused;
      uint16_t mtu;
    } frag;
  } un;
};
#endif
#endif

/* User Data added to the ICMP header
 *
 * The 'ts' is the time the request is sent on the wire
 * and it is used to compute the network round-trip value.
 *
 * The 'index' parameter is an index value in the array of hosts to ping
 * and it is used to relate each response with the corresponding request
 */
struct evdata {
  uint64_t ts;
  uint32_t dest_ip;
};

#define IPHDR 20
#define MIN_DATA_SIZE sizeof(struct evdata)
// #define DEFAULT_DATA_SIZE    (MIN_DATA_SIZE + 44)            /*
// calculated as so to be like traditional ping */
#define DEFAULT_DATA_SIZE (MIN_DATA_SIZE + 0) /* calculated as so to be like traditional ping */
#define MAX_DATA_SIZE (IP_MAXPACKET - IPHDR - ICMP_MINLEN)
#define DEFAULT_PKT_SIZE ICMP_MINLEN + DEFAULT_DATA_SIZE

/* Intervals and timeouts (all are in milliseconds unless otherwise specified)
 */
#define DEFAULT_NOREPLY_TIMEOUT 5000 /* 100 msec - 0 is illegal      */
#define DEFAULT_PING_INTERVAL 2000   /* 1 sec - 0 means flood mode   */

#if defined(_WIN32) || defined(__APPLE__)
typedef uint16_t n_short;
typedef uint32_t n_long; /* long as received from the net */
typedef uint32_t n_time; /* ms since 00:00 GMT, byte rev */

#ifdef __STDC__
#define BITFIELD_UINT8 uint8_t
#else
#define BITFIELD_UINT8 uint8_t
#endif
struct ip {
  BITFIELD_UINT8
#if BYTE_ORDER == LITTLE_ENDIAN
  ip_hl : 4,    /* header length */
      ip_v : 4; /* version */
#endif
  // #if BYTE_ORDER == BIG_ENDIAN
  // ip_v : 4,                   /* version */
  // ip_hl : 4;                  /* header length */
  // #endif
  uint8_t ip_tos;                /* type of service */
  uint16_t ip_len;               /* total length */
  uint16_t ip_id;                /* identification */
  uint16_t ip_off;               /* flags and fragment offset */
#define IP_DF 0x4000             /* dont fragment flag */
#define IP_MF 0x2000             /* more fragments flag */
#define IP_OFFMASK 0x1fff        /* mask for fragmenting bits */
  uint8_t ip_ttl;                /* time to live */
  uint8_t ip_p;                  /* protocol */
  uint16_t ip_sum;               /* checksum */
  struct in_addr ip_src, ip_dst; /* source and dest address */
};

struct icmp {
  uint8_t icmp_type;   /* type of message, see below */
  uint8_t icmp_code;   /* type sub code */
  uint16_t icmp_cksum; /* ones complement cksum of struct */
  union {
    uint8_t ih_pptr;          /* ICMP_PARAMPROB */
    struct in_addr ih_gwaddr; /* ICMP_REDIRECT */
    struct ih_idseq {
      n_short icd_id;
      n_short icd_seq;
    } ih_idseq;
    int32_t ih_void;

    /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
    struct ih_pmtu {
      n_short ipm_void;
      n_short ipm_nextmtu;
    } ih_pmtu;
  } icmp_hun;
#define icmp_pptr icmp_hun.ih_pptr
#define icmp_gwaddr icmp_hun.ih_gwaddr
#define icmp_id icmp_hun.ih_idseq.icd_id
#define icmp_seq icmp_hun.ih_idseq.icd_seq
#define icmp_void icmp_hun.ih_void
#define icmp_pmvoid icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu icmp_hun.ih_pmtu.ipm_nextmtu
  union {
    struct id_ts {
      n_time its_otime;
      n_time its_rtime;
      n_time its_ttime;
    } id_ts;
    struct id_ip {
      struct ip idi_ip;
      /* options and then 64 bits of data */
    } id_ip;
    uint32_t id_mask;
    char id_data[1];
  } icmp_dun;
#define icmp_otime icmp_dun.id_ts.its_otime
#define icmp_rtime icmp_dun.id_ts.its_rtime
#define icmp_ttime icmp_dun.id_ts.its_ttime
#define icmp_ip icmp_dun.id_ip.idi_ip
#define icmp_mask icmp_dun.id_mask
#define icmp_data icmp_dun.id_data
};

/*
 * Definition of type and code field values.
 */
#define ICMP_ECHOREPLY 0            /* echo reply */
#define ICMP_UNREACH 3              /* dest unreachable, codes: */
#define ICMP_UNREACH_NET 0          /* bad net */
#define ICMP_UNREACH_HOST 1         /* bad host */
#define ICMP_UNREACH_PROTOCOL 2     /* bad protocol */
#define ICMP_UNREACH_PORT 3         /* bad port */
#define ICMP_UNREACH_NEEDFRAG 4     /* IP_DF caused drop */
#define ICMP_UNREACH_SRCFAIL 5      /* src route failed */
#define ICMP_UNREACH_NET_UNKNOWN 6  /* unknown net */
#define ICMP_UNREACH_HOST_UNKNOWN 7 /* unknown host */
#define ICMP_UNREACH_ISOLATED 8     /* src host isolated */
#define ICMP_UNREACH_NET_PROHIB 9   /* prohibited access */
#define ICMP_UNREACH_HOST_PROHIB 10 /* ditto */
#define ICMP_UNREACH_TOSNET 11      /* bad tos for net */
#define ICMP_UNREACH_TOSHOST 12     /* bad tos for host */
#define ICMP_SOURCEQUENCH 4         /* packet lost, slow down */
#define ICMP_REDIRECT 5             /* shorter route, codes: */
#define ICMP_REDIRECT_NET 0         /* for network */
#define ICMP_REDIRECT_HOST 1        /* for host */
#define ICMP_REDIRECT_TOSNET 2      /* for tos and net */
#define ICMP_REDIRECT_TOSHOST 3     /* for tos and host */
#define ICMP_ECHO 8                 /* echo service */
#define ICMP_ROUTERADVERT 9         /* router advertisement */
#define ICMP_ROUTERSOLICIT 10       /* router solicitation */
#define ICMP_TIMXCEED 11            /* time exceeded, codes: */
#define ICMP_TIMXCEED_INTRANS 0     /* ttl==0 in transit */
#define ICMP_TIMXCEED_REASS 1       /* ttl==0 in reass */
#define ICMP_PARAMPROB 12           /* ip header bad */
#define ICMP_PARAMPROB_OPTABSENT 1  /* req. opt. absent */
#define ICMP_TSTAMP 13              /* timestamp request */
#define ICMP_TSTAMPREPLY 14         /* timestamp reply */
#define ICMP_IREQ 15                /* information request */
#define ICMP_IREQREPLY 16           /* information reply */
#define ICMP_MASKREQ 17             /* address mask request */
#define ICMP_MASKREPLY 18           /* address mask reply */

#endif

using std::placeholders::_1;

namespace agora {
namespace commons {

ping_client_libevent::ping_client_libevent(io_engine_factory* factory, io_engine_base* engine,
                                           sink_type&& sink, int timeout)
    : net_(*static_cast<libevent::event_engine*>(engine)),
      e_(nullptr),
      sink_(std::move(sink)),
      timeout_(timeout),
      using_ping_socket_(false) {
  initialize();
}

ping_client_libevent::~ping_client_libevent() {
  if (e_) {
    evutil_socket_t s = ::event_get_fd(e_);
    ::event_free(e_);
    ::evutil_closesocket(s);
  }
}

int ping_client_libevent::initialize() {
  uint16_t ident = decltype(ident)((uint64_t)(this));

  /* Create an endpoint for communication using raw socket for ICMP calls */
  evutil_socket_t s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (s != -1) {
    using_ping_socket_ = true;
  } else {
    using_ping_socket_ = false;
    s = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  }
  if (s == -1) {
    log(LOG_WARN, "create socket error: %s", socket_error_to_string(socket_error()));
    return -EIO;
  }

  ::evutil_make_socket_nonblocking(s);
#if defined(__APPLE__)
  int set = 1;
  setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<void*>(&set), sizeof(int));
#elif defined(__linux__)
  if (using_ping_socket_) {
    struct sockaddr_in sa;
    socklen_t sl;
    sa.sin_family = AF_INET;
    sa.sin_port = 0;
    sa.sin_addr.s_addr = 0;
    sl = sizeof(sa);
    if (bind(s, (struct sockaddr*)&sa, sl) == -1) {
      log(LOG_WARN, "bind error: %s", socket_error_to_string(socket_error()));
      ::evutil_closesocket(s);
      return -EIO;
    }
    if (getsockname(s, (struct sockaddr*)&sa, &sl) == -1) {
      log(LOG_WARN, "getsockname error: %s", socket_error_to_string(socket_error()));
      ::evutil_closesocket(s);
      return -EIO;
    }
    ident = sa.sin_port;

    int hold = 1;
    if (setsockopt(s, SOL_IP, IP_RECVTTL, reinterpret_cast<char*>(&hold), sizeof(hold))) {
      ::evutil_closesocket(s);
      return -EIO;
    }
    if (setsockopt(s, SOL_IP, IP_RETOPTS, reinterpret_cast<char*>(&hold), sizeof(hold))) {
      ::evutil_closesocket(s);
      return -EIO;
    }
  }
#endif

  memset(&data_, 0, sizeof(data_));
  data_.ident = ident;
  data_.pktsize = DEFAULT_PKT_SIZE;
  data_.noreply_interval = DEFAULT_NOREPLY_TIMEOUT;
  data_.ping_interval = timeout_;

  /* Define the callback to handle ICMP Echo Reply and add the raw file
   * descriptor to those monitored for read events */
  e_ = ::event_new(net_.engine_handle(), s, EV_READ | EV_PERSIST, ping_data_callback, this);
  ::event_add(e_, nullptr);
  return 0;
}

ping_client_libevent::evhost::evhost() { clear(); }

ping_client_libevent::evhost::evhost(ping_client_libevent::evhost&& rhs)
    : dest_ip(std::move(rhs.dest_ip)),
      seq(rhs.seq),
      noreply_timer(std::move(rhs.noreply_timer)),
      ping_timer(std::move(rhs.ping_timer)),
      sentpkts(rhs.sentpkts),
      recvpkts(rhs.recvpkts),
      dropped(rhs.dropped),
      sentbytes(rhs.sentbytes),
      recvbytes(rhs.recvbytes),
      firstsent(rhs.firstsent),
      firstrecv(rhs.firstrecv),
      lastsent(rhs.lastsent),
      lastrecv(rhs.lastrecv),
      shortest(rhs.shortest),
      longest(rhs.longest),
      sum(rhs.sum),
      square(rhs.square) {}

void ping_client_libevent::evhost::clear() {
  dest_ip = ip_t();
  seq = 0;

  sentpkts = recvpkts = dropped = 0;
  sentbytes = recvbytes = 0;
  firstsent = firstrecv = lastsent = lastrecv = 0;
  shortest = longest = sum = square = 0;
}

int ping_client_libevent::add_address(const ip_t& dest_ip, int interval,
                                      const ip_t* local_address) {
  if (!ip::is_valid(dest_ip)) return -EINVAL;
  if (!e_) return -ENODEV;
  auto* h = find_host(dest_ip);
  if (h) {
    // already exist
    h->ping_timer.reset(net_.create_timer(std::bind(&ping_client_libevent::on_ping_timer, this, h),
                                          interval, false));
    return 0;
  }

  log(LOG_INFO, "add ping address %s", ip::to_string(dest_ip).c_str());

  evhost host;
  host.dest_ip = dest_ip;
  host.seq = 1;
  host.shortest = -1;

  hosts_.push_back(std::move(host));

  evhost& hb = hosts_.back();
  /* Define here the callbacks to ping the host and to handle no reply timeouts
   */
  hb.ping_timer.reset(net_.create_timer(std::bind(&ping_client_libevent::on_ping_timer, this, &hb),
                                        interval, false));
  hb.noreply_timer.reset(net_.create_timer(std::bind(&ping_client_libevent::on_noreply, this, &hb),
                                           data_.noreply_interval, false));
  on_ping_timer(&hb);
  return 0;
}

int ping_client_libevent::remove_address(const ip_t& dest_ip) {
  if (!ip::is_valid(dest_ip)) return -1;

  for (auto it = hosts_.begin(); it != hosts_.end(); ++it) {
    evhost& h = *it;
    if (h.dest_ip == dest_ip) {
      hosts_.erase(it);
      return 0;
    }
  }
  return -1;
}

int ping_client_libevent::remove_all_addresses() {
  hosts_.clear();
  return 0;
}

void ping_client_libevent::stats() {
  for (auto& h : hosts_) {
    auto* host = &h;
    log(LOG_INFO, "--- %s ping statistics ---", ip::to_string(host->dest_ip).c_str());
    log(LOG_INFO,
        "%llu packets transmitted, %llu received, %.2f%% packet loss, time "
        "%dms",
        host->sentpkts, host->recvpkts, 100.0 * (host->sentpkts - host->recvpkts) / host->sentpkts,
        host->sum);

    if (host->recvpkts) {
      uint32_t average = (uint32_t)(host->sum / host->recvpkts);
      uint32_t deviation =
          (uint32_t)sqrt(((host->recvpkts * host->square) - (host->sum * host->sum)) /
                         (host->recvpkts * (host->recvpkts - 1)));

      log(LOG_INFO, "rtt min/avg/max/sdev = %d/%d/%d/%d ms", host->shortest, average, host->longest,
          deviation);
    }
  }
}

/* exported function */
const char* ping_client_libevent::error_to_string(int err) {
  switch (err) {
    case PING_ERR_NONE:
      return "no error";
    case PING_ERR_TIMEOUT:
      return "request timed out";
    case PING_ERR_SHUTDOWN:
      return "ping subsystem shut down";
    case PING_ERR_CANCEL:
      return "ping request canceled";
    case PING_ERR_UNKNOWN:
      return "unknown";
    default:
      return "[Unknown error code]";
  }
}

void ping_client_libevent::ping_data_callback(evutil_socket_t fd, int16_t event, void* context) {
  ping_client_libevent* thiz = reinterpret_cast<ping_client_libevent*>(context);
  thiz->on_data(fd, event);
}

ping_client_libevent::evhost* ping_client_libevent::find_host(const ip_t& dest_ip) {
  for (auto it = hosts_.begin(); it != hosts_.end(); ++it) {
    auto& h = *it;
    if (h.dest_ip == dest_ip) {
      return &h;
    }
  }
  return nullptr;
}

void ping_client_libevent::on_data(evutil_socket_t fd, int16_t event) {
  evping* base = &data_;

  int nrecv;
  char packet[MAX_DATA_SIZE];
  struct sockaddr_in remote; /* responding internet address */
  ev_socklen_t slen = sizeof(struct sockaddr);

  /* Pointer to relevant portions of the packet (IP, ICMP and user data) */
  struct ip* ip = (struct ip*)packet;
  struct icmphdr* icmp;
  struct evdata* data = (struct evdata*)(packet + IPHDR + ICMP_MINLEN);
  int hlen = 0;

  /* Time the packet has been received */
  uint64_t now = tick_ms();

  /* Receive data from the network */
  nrecv = recvfrom(fd, packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr*)&remote, &slen);
  if (nrecv < 0) {
    /* One more failure */
    base->recvfail++;
    return;
  }

  /* One more ICMP packet received */
  base->recvok++;

#if defined(__APPLE__)
  bool has_ip_header = true;
  int pktsize = base->pktsize + IPHDR;
#else
  bool has_ip_header = !using_ping_socket_;
  int pktsize = base->pktsize;
#endif
  /* Calculate the IP header length */
  if (has_ip_header) {
    hlen = ip->ip_hl * 4;

    /* Check the IP header */
    if (nrecv < hlen + ICMP_MINLEN || ip->ip_hl < 5) {
      /* One more too short packet */
      base->tooshort++;
      return;
    }
  } else {
    hlen = 0;
    data = (struct evdata*)(packet + ICMP_MINLEN);
  }

  /* The ICMP portion */
  icmp = (struct icmphdr*)(packet + hlen);

  /* Check the ICMP header to drop unexpected packets due to unrecognized id */
  if (icmp->un.echo.id != base->ident || nrecv != pktsize) {
    /* One more foreign packet */
    base->foreign++;
    return;
  }

  auto* host = find_host(ip::from_string(ipv4::to_string(data->dest_ip)));
  /* Check the ICMP payload for legal values of the 'index' portion */
  if (!host) {
    /* One more illegal packet */
    base->illegal++;
    return;
  }

  /* Check for Destination Host Unreachable */
  if (icmp->type == ICMP_ECHOREPLY) {
    /* Compute time difference to calculate the round trip */
    uint32_t elapsed = (uint32_t)(now - data->ts);

    /* Update timestamps */
    if (!host->recvpkts) host->firstrecv = now;
    host->lastrecv = now;
    host->recvpkts++;
    host->recvbytes += nrecv;

    /* Update counters */
    host->shortest = (std::min)(host->shortest, elapsed);
    host->longest = (std::max)(host->longest, elapsed);
    host->sum += elapsed;
    host->square += (elapsed * elapsed);

    if (sink_) sink_(host->dest_ip, PING_ERR_NONE, elapsed);

    /* Update the sequence number for the next run */
    host->seq = (host->seq + 1) % 256;

    /* Clean the no-reply timer */
    host->noreply_timer->cancel();

    /* Add the timer to ping again the host at the given time interval */
    host->ping_timer->schedule(base->ping_interval);
  } else {
    /* Handle this condition exactly as the request has expired */
    on_noreply(host);
  }
}

/* The callback to handle timeouts due to destination host unreachable condition
 */
void ping_client_libevent::on_noreply(evhost* host) {
  evping* base = &data_;
  host->dropped++;

  /* Add the timer to ping again the host at the given time interval */
  host->ping_timer->schedule(base->ping_interval);

  if (sink_) sink_(host->dest_ip, PING_ERR_TIMEOUT, base->noreply_interval);

  /* Update the sequence number for the next run */
  host->seq = (host->seq + 1) % 256;
}

/*
 * Checksum routine for Internet Protocol family headers (C Version).
 * From ping examples in W. Richard Stevens "Unix Network Programming" book.
 */
static uint16_t mkcksum(uint16_t* p, int n) {
  uint16_t answer;
  int32_t sum = 0;
  uint16_t odd_byte = 0;

  while (n > 1) {
    sum += *p++;
    n -= 2;
  }

  /* mop up an odd byte, if necessary */
  if (n == 1) {
    *reinterpret_cast<uint8_t*>(&odd_byte) = *(reinterpret_cast<uint8_t*>(p));
    sum += odd_byte;
  }

  sum = (sum >> 16) + (sum & 0xffff); /* add high 16 to low 16 */
  sum += (sum >> 16);                 /* add carry */
  answer = (uint16_t)~sum;            /* ones-complement, truncate */

  return answer;
}

/*
 * Format an ICMP Echo Request packet to be sent over the wire.
 *
 *  o the IP packet will be added on by the kernel
 *  o the ID field is the Unix process ID
 *  o the sequence number is an ascending integer
 *
 * The first 8 bytes of the data portion are used
 * to hold a Unix "timeval" struct in VAX byte-order,
 * to compute the network round-trip value.
 *
 * The second 8 bytes of the data portion are used
 * to keep an unique integer used as index in the array
 * ho hosts being monitored
 */
static void fmticmp(char* buffer, unsigned size, uint8_t seq, const ip_t& dest_ip, uint16_t ident) {
  memset(buffer, 0, size);
  struct icmp* icmp = (struct icmp*)buffer;
  struct evdata* data = (struct evdata*)(buffer + ICMP_MINLEN);

  /* The ICMP header (no checksum here until user data has been filled in) */
  icmp->icmp_type = ICMP_ECHO; /* type of message */
  icmp->icmp_code = 0;         /* type sub code */
  icmp->icmp_id = ident;       /* unique process identifier */
  icmp->icmp_seq = seq;        /* message identifier */

  /* User data */
  data->ts = tick_ms(); /* current time */
  data->dest_ip = ipv4::from_string(ip::to_string(dest_ip));

  /* Last, compute ICMP checksum */
  icmp->icmp_cksum =
      mkcksum(reinterpret_cast<uint16_t*>(icmp), size); /* ones complement checksum of struct */
}

void ping_client_libevent::on_ping_timer(evhost* host) {
  if (!e_) return;

  evping* base = &data_;

  char packet[MAX_DATA_SIZE];
  int nsent;

  /* Clean the no reply timer (if any was previously set) */
  host->noreply_timer->cancel();

  /* Format the ICMP Echo Reply packet to send */
  fmticmp(packet, base->pktsize, host->seq, host->dest_ip, base->ident);
  ip::sockaddr_t addr = ip::to_address(host->dest_ip, 0);
  /* Transmit the request over the network */
  evutil_socket_t s = event_get_fd(e_);
  nsent = sendto(s, packet, base->pktsize, MSG_DONTWAIT, &addr.sa, sizeof(struct sockaddr_in));

  if (nsent == base->pktsize) {
    /* One more ICMP Echo Request sent */
    base->sentok++;

    if (!host->sentpkts && !base->quiet)
      log(LOG_INFO, "PING %s %d(%d) bytes of data", ip::to_string(addr).c_str(),
          base->pktsize - ICMP_MINLEN, nsent + IPHDR);

    uint64_t now = tick_ms();
    /* Update timestamps and counters */
    if (!host->sentpkts) host->firstsent = now;
    host->lastsent = now;
    host->sentpkts++;
    host->sentbytes += nsent;

    /* Add the timer to handle no reply condition in the given timeout */
    host->noreply_timer->schedule(base->noreply_interval);
  } else {
    const char* err = socket_error_to_string(socket_error());
    base->sendfail++;
  }
}

}  // namespace commons
}  // namespace agora
