/**
 * THREADING MODES
 *
 *     single_threaded             - Your program is assumed to be single threaded from the point of
 * view of signal/slot usage (i.e. all objects using signals and slots are created and destroyed
 * from a single thread). Behaviour if objects are destroyed concurrently is undefined (i.e. you'll
 * get the occasional segmentation fault/memory exception).
 *
 *     multi_threaded_global       - Your program is assumed to be multi threaded. Objects using
 * signals and slots can be safely created and destroyed from any thread, even when connections
 * exist. In multi_threaded_global mode, this is achieved by a single global mutex (actually a
 * critical section on Windows because they are faster). This option uses less OS resources, but
 * results in more opportunities for contention, possibly resulting in more context switches than
 * are strictly necessary.
 *
 *     multi_threaded_local        - Behaviour in this mode is essentially the same as
 * multi_threaded_global, except that each signal, and each object that inherits has_slots, all have
 * their own mutex/critical section. In practice, this means that mutex collisions (and hence
 * context switches) only happen if they are absolutely essential. However, on some platforms,
 * creating a lot of mutexes can slow down the whole OS, so use this option with care.
 *
 * USING THE LIBRARY
 *     See the full documentation at http://sigslot.sourceforge.net/
 *
 */

#ifndef _SIGSLOT_H__
#define _SIGSLOT_H__

#include <assert.h>
#include <functional>
#include <list>
#include <mutex>
#include <set>
#include <thread>

#include "utils/log/log.h"

#ifndef _SIGSLOT_MULTI_THREADED
#define AGORA_SIGSLOT_DEFAULT_MT_POLICY agora::thread::st
#else
#define AGORA_SIGSLOT_DEFAULT_MT_POLICY agora::thread::mt
#endif
#define SIGSLOT_ENABLE_THREAD_AFFINITY 1

namespace agora {
namespace thread {
class st  // Single threaded
{
 public:
  st() {}
  virtual ~st() {}
  void lock() {}
  void unlock() {}
  void test(st const*) const {}
};

class mtg {
 public:
  mtg() {}
  virtual ~mtg() {}
  void lock() { get_mutex()->lock(); }
  void unlock() { get_mutex()->unlock(); }
  void test(mtg const*) const {}

 private:
  mtg(const mtg&);
  std::mutex* get_mutex() {
    static std::mutex g_mutex;
    return &g_mutex;
  }
};

class mt {
 public:
  mt() {}
  virtual ~mt() {}
  void lock() { m_mutex.lock(); }
  void unlock() { m_mutex.unlock(); }
  void test(mt const*) const {}

 private:
  mt(const mt&);
  std::mutex m_mutex;
};

}  // namespace thread

template <class mt_policy>
class has_slots;

namespace internal {
template <class mt_policy>
class lock_block {
 public:
  mt_policy* m_mutex;
  lock_block(mt_policy* mtx) : m_mutex(mtx) { m_mutex->lock(); }
  ~lock_block() { m_mutex->unlock(); }
};

template <class mt_policy, class... args>
class _connection_base {
 public:
  virtual ~_connection_base() {}
  virtual has_slots<mt_policy>* getdest() const = 0;
  virtual void emit(args...) = 0;
  virtual _connection_base* clone() = 0;
  virtual _connection_base* duplicate(has_slots<mt_policy>* pnewdest) = 0;
};

template <class mt_policy>
class _signal_base_lo  // : public mt_policy
{
 public:
  virtual void slot_disconnect(has_slots<mt_policy>* pslot) = 0;
  virtual void slot_duplicate(const has_slots<mt_policy>* poldslot,
                              has_slots<mt_policy>* pnewslot) = 0;
  mt_policy m_policy;
};

template <class mt_policy, class... args>
class _signal_base : public _signal_base_lo<mt_policy> {
 public:
  typedef typename std::list<_connection_base<mt_policy, args...>*> connections_list;

  _signal_base() {}
  ~_signal_base() { disconnect_all(); }

  void disconnect_all() {
    lock_block<mt_policy> lock(&(this->m_policy));
    for (auto i : m_connected_slots) {
      i->getdest()->signal_disconnect(this);
      delete i;
    }
    m_connected_slots.clear();
  }

  void disconnect(has_slots<mt_policy>* pclass) {
    lock_block<mt_policy> lock(&(this->m_policy));
    bool found{false};
    m_connected_slots.remove_if([pclass, &found](_connection_base<mt_policy, args...>* x) {
      if (x->getdest() == pclass) {
        delete x;
        found = true;
        return true;
      }
      return false;
    });
    if (found) pclass->signal_disconnect(this);
  }

  void slot_disconnect(has_slots<mt_policy>* pslot) {
    lock_block<mt_policy> lock(&(this->m_policy));
    m_connected_slots.remove_if([pslot](_connection_base<mt_policy, args...>* x) {
      if (x->getdest() == pslot) {
        delete x;
        return true;
      }
      return false;
    });
  }

  void slot_duplicate(const has_slots<mt_policy>* oldtarget, has_slots<mt_policy>* newtarget) {
    lock_block<mt_policy> lock(&(this->m_policy));
    for (auto i : m_connected_slots) {
      if (i->getdest() == oldtarget) {
        m_connected_slots.push_back(i->duplicate(newtarget));
      }
    }
  }

  size_t size() const { return m_connected_slots.size(); }

 protected:
  connections_list m_connected_slots;
  mt_policy m_policy;

 private:
  _signal_base(const _signal_base& s);
};

template <class dest_type, class mt_policy, class... args>
class _connection : public _connection_base<mt_policy, args...> {
 public:
  _connection(dest_type* pobject, const std::function<void(args...)>& fn)
      : m_pobject(pobject), m_fn(fn) {}
  _connection(dest_type* pobject, std::function<void(args...)>&& fn)
      : m_pobject(pobject), m_fn(std::move(fn)) {}

  virtual _connection_base<mt_policy, args...>* clone() {
    return new _connection<dest_type, mt_policy, args...>(*this);
  }

  virtual _connection_base<mt_policy, args...>* duplicate(has_slots<mt_policy>* pnewdest) {
    return new _connection<dest_type, mt_policy, args...>((dest_type*)pnewdest, m_fn);
  }

  virtual void emit(args... a) { m_fn(a...); }

  virtual has_slots<mt_policy>* getdest() const { return m_pobject; }

 private:
  dest_type* m_pobject;
  std::function<void(args...)> m_fn;
};

}  // namespace internal

template <class mt_policy = AGORA_SIGSLOT_DEFAULT_MT_POLICY>
class has_slots  // : public mt_policy
{
 private:
  typedef typename std::set<internal::_signal_base_lo<mt_policy>*> sender_set;

 public:
  has_slots() { ; }

  has_slots(const has_slots& hs) {
    internal::lock_block<mt_policy> lock(&m_policy);
    for (auto i : hs.m_senders) {
      i->slot_duplicate(&hs, this);
      m_senders.insert(i);
    }
  }

  void signal_connect(internal::_signal_base_lo<mt_policy>* sender) {
    internal::lock_block<mt_policy> lock(&m_policy);
    m_senders.insert(sender);
  }

  void signal_disconnect(internal::_signal_base_lo<mt_policy>* sender) {
    internal::lock_block<mt_policy> lock(&m_policy);
    m_senders.erase(sender);
  }

  virtual ~has_slots() { disconnect_all(); }

  void disconnect_all() {
    internal::lock_block<mt_policy> lock(&m_policy);
    for (auto i : m_senders) {
      i->slot_disconnect(this);
    }

    m_senders.clear();
  }
  const mt_policy* policy() const { return &m_policy; }

 private:
  sender_set m_senders;
  mt_policy m_policy;
};

template <typename mt_policy = AGORA_SIGSLOT_DEFAULT_MT_POLICY, class... args>
class signal : public internal::_signal_base<mt_policy, args...> {
 public:
  typedef typename internal::_signal_base<mt_policy, args...>::connections_list::const_iterator
      const_iterator;
  signal() {
#if defined(SIGSLOT_ENABLE_THREAD_AFFINITY)
    set_thread_affinity();
#endif
  }
  template <class desttype>
  void connect(desttype* pclass, std::function<void(args...)>&& fn) {
    this->m_policy.test(pclass->policy());  // Ensure it's the same mt_policy.
    internal::lock_block<mt_policy> lock(&(this->m_policy));
    internal::_connection<desttype, mt_policy, args...>* conn =
        new internal::_connection<desttype, mt_policy, args...>(pclass, std::move(fn));
    this->m_connected_slots.push_back(conn);
    pclass->signal_connect(this);
  }

  // Helper for ptr-to-member; call the member function "normally".
  template <class desttype>
  void connect(desttype* pclass, void (desttype::*memfn)(args...)) {
    this->connect(pclass, [pclass, memfn](args... a) { (pclass->*memfn)(a...); });
  }

  // This code uses the long-hand because it assumes it may mutate the list.
  void emit(args... a) {
#if defined(SIGSLOT_ENABLE_THREAD_AFFINITY)
    check_thread_affinity();
#endif
    internal::lock_block<mt_policy> lock(&(this->m_policy));
    for (const auto& slot : this->m_connected_slots) {
      slot->emit(a...);
    }
  }
#if 0
		void emit(bool debug, args... a)
		{
			agora::commons::log(agora::commons::LOG_INFO, "[%p] emit signal, %d slots", this, (int)this->size());
			internal::lock_block<mt_policy> lock(&(this->m_policy));
			for (const auto& slot : this->m_connected_slots)
			{
				agora::commons::log(agora::commons::LOG_INFO, "[%p] emit signal to slot %p", this, slot);
				slot->emit(a...);
			}
			agora::commons::log(agora::commons::LOG_INFO, "[%p] done emit signal, %d slots", this);
		}
#endif
  void operator()(args... a) { this->emit(a...); }
#if defined(SIGSLOT_ENABLE_THREAD_AFFINITY)
  void set_thread_affinity() { thread_id = std::this_thread::get_id(); }
  void check_thread_affinity() {
    // must call from the same thread
    if (thread_id != std::this_thread::get_id()) assert(false);
  }

 private:
  std::thread::id thread_id;
#endif
};

template <typename mt_policy = AGORA_SIGSLOT_DEFAULT_MT_POLICY, class... args>
class repeater : public signal<mt_policy, args...>, public has_slots<mt_policy> {
 public:
  typedef signal<mt_policy, args...> base_type;
  typedef repeater<mt_policy, args...> this_type;

  repeater() {}
  repeater(const this_type& s) : base_type(s) {}

  void reemit(args... a) { base_type::emit(a...); }
  void repeat(base_type& s) { s.connect(this, &this_type::reemit); }
};

template <class... args>
struct signal_type {
  typedef signal<AGORA_SIGSLOT_DEFAULT_MT_POLICY, args...> sig;
  typedef repeater<AGORA_SIGSLOT_DEFAULT_MT_POLICY, args...> rep;
};

}  // namespace agora

#endif  // _SIGSLOT_H__
