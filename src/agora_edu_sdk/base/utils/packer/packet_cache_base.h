#pragma once

#include <cassert>
#include <list>
#include <unordered_map>

namespace agora {
namespace commons {
template <typename Key, typename Value>
class packet_cache {
  struct value_type;

  typedef Key key_t;
  typedef Value value_t;
  typedef std::list<value_type> container_t;
  typedef typename container_t::iterator container_iter_t;
  typedef std::unordered_map<Key, container_iter_t> track_map_t;
  typedef typename track_map_t::iterator key_iter_t;

  struct value_type {
      typename packet_cache::key_t key;
      typename packet_cache::value_t value;

    template <typename V>
    value_type(typename packet_cache::key_t k, V &&val) :key(k), value(std::forward<V>(val)) {}
  };
 public:
  explicit packet_cache(size_t size=65536);
  packet_cache(packet_cache && rhs)
      :cache_size_(rhs.cache_size_)
      , keys_(std::move(rhs.keys_))
      , items_(std::move(rhs.items_)) {
  }
  packet_cache& operator=(packet_cache && rhs) {
      if (this != &rhs) {
          cache_size_ = rhs.cache_size_;
          keys_ = std::move(rhs.keys_);
          items_ = std::move(rhs.items_);
      }
      return *this;
  }

  //~packet_cache() = default;

  bool get_entry(const Key &key, const Value **v);
  template <typename V> void set_entry(const Key &key, V &&v);

  template <typename Callable> void shrink_until(Callable &&f);
  size_t size() const;
 private:
  packet_cache(const packet_cache &) = delete;
  packet_cache& operator=(packet_cache &) = delete;
 private:
  const size_t cache_size_;

  track_map_t keys_;
  container_t items_;
};

template <typename Key, typename Value>
packet_cache<Key, Value>::packet_cache(size_t size) :cache_size_(size) {
}

template <typename Key, typename Value>
template <typename Callable>
void packet_cache<Key, Value>::shrink_until(Callable &&f) {
  // NOTE(liuyong): pop_front donnot invalidate the iterators stored in |keys|
  while (items_.begin() != items_.end() && !f(items_.front().value)) {
    keys_.erase(items_.front().key);
    items_.pop_front();
  }
}

template <typename Key, typename Value>
bool packet_cache<Key, Value>::get_entry(const Key &key, const Value **v) {
  auto it = keys_.find(key);
  if (it == keys_.end())
    return false;

  *v = &(*it->second).value;
  return true;
}

template <typename Key, typename Value>
template <typename V>
void packet_cache<Key, Value>::set_entry(const Key &k, V &&v) {
  key_iter_t f = keys_.find(k);
  if (f == keys_.end()) {
    if (keys_.size() >= cache_size_) {
      keys_.erase(items_.front().key);
      items_.pop_front();
    }

    items_.emplace_back(k, std::forward<V>(v));
    std::pair<key_iter_t, bool> p = keys_.emplace(k, --items_.end());
    assert(p.second);
  } else {
    value_type &val = *f->second;
    val.value = std::forward<V>(v);
  }
}

template <typename Key, typename Value>
inline size_t packet_cache<Key, Value>::size() const {
  return keys_.size();
}

}
}
