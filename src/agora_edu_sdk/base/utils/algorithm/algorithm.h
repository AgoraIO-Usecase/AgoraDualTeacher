//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <algorithm>
#include <vector>
#include "utils/log/log.h"

namespace agora {
namespace commons {

struct pair_hasher {
  size_t operator()(const std::pair<uint32_t, uint32_t> &a) const {
    union {
      uint64_t n;
      struct {
        uint32_t high;
        uint32_t low;
      } bits;
    };

    bits.high = a.first;
    bits.low = a.second;
    return hasher_(n);
  }

 private:
  std::hash<uint64_t> hasher_;
};

template <class InputIterator, class OutputIterator, class UnaryFunction, class Predicate>
OutputIterator transform_if(InputIterator first, InputIterator last, OutputIterator result,
                            UnaryFunction f, Predicate pred) {
  for (; first != last; ++first) {
    if (pred(*first)) *result++ = f(*first);
  }
  return result;
}

template <typename Container, typename Fn1>
inline void retain(Container &c, Fn1 f) {
  typename Container::iterator it = c.begin();
  while (it != c.end()) {
    if (f(*it)) {
      ++it;
    } else {
      it = c.erase(it);
    }
  }
}

template <typename Container, typename Fn1>
inline void retain_till(Container &c, Fn1 f) {
  typename Container::iterator it = c.begin();
  while (it != c.end()) {
    if (f(*it)) {
      return;
    }
    it = c.erase(it);
  }
}

template <typename ElementType, typename Fn1>
inline void retain_till(std::vector<ElementType> &c, Fn1 f) {
  c.erase(c.begin(), std::find_if(c.begin(), c.end(), f));
}

template <typename ElementType>
inline ElementType average_least(const std::vector<ElementType> &numbers, size_t least) {
  if (numbers.empty()) {
    return 0;
  }

  std::vector<ElementType> sorted = numbers;
  std::sort(sorted.begin(), sorted.end());
  ElementType total = 0;
  size_t count = min(least, sorted.size());
  for (size_t i = 0; i < count; ++i) {
    total += sorted[i];
  }
  return total / count;
}
}  // namespace commons
}  // namespace agora
