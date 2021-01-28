//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include "utils/strings/string_piece.h"

#include <limits.h>
#include <memory.h>

#include <algorithm>
#include <ostream>

namespace agora {
namespace utils {
namespace {

// For each character in characters_wanted, sets the index corresponding
// to the ASCII code of that character to 1 in table.  This is used by
// the find_.*_of methods below to tell whether or not a character is in
// the lookup table in constant time.
// The argument `table' must be an array that is large enough to hold all
// the possible values of an unsigned char.  Thus it should be be declared
// as follows:
//   bool table[UCHAR_MAX + 1]
inline void BuildLookupTable(const StringPiece& characters_wanted, bool* table) {
  const size_t length = characters_wanted.length();
  const char* const data = characters_wanted.data();
  for (size_t i = 0; i < length; ++i) {
    table[static_cast<unsigned char>(data[i])] = true;
  }
}

}  // namespace

// MSVC doesn't like complex extern templates and DLLs.
#if !defined(COMPILER_MSVC)
template class BasicStringPiece<std::string>;
#endif

bool operator==(const StringPiece& x, const StringPiece& y) {
  if (x.size() != y.size()) return false;

  return CharTraits<StringPiece::value_type>::compare(x.data(), y.data(), x.size()) == 0;
}

std::ostream& operator<<(std::ostream& o, const StringPiece& piece) {
  o.write(piece.data(), static_cast<std::streamsize>(piece.size()));
  return o;
}

namespace internal {

template <typename STR>
void CopyToStringT(const BasicStringPiece<STR>& self, STR* target) {
  if (self.empty())
    target->clear();
  else
    target->assign(self.data(), self.size());
}

void CopyToString(const StringPiece& self, std::string* target) { CopyToStringT(self, target); }

template <typename STR>
void AppendToStringT(const BasicStringPiece<STR>& self, STR* target) {
  if (!self.empty()) target->append(self.data(), self.size());
}

void AppendToString(const StringPiece& self, std::string* target) { AppendToStringT(self, target); }

template <typename STR>
size_t copyT(const BasicStringPiece<STR>& self, typename STR::value_type* buf, size_t n,
             size_t pos) {
  size_t ret = std::min(self.size() - pos, n);
  memcpy(buf, self.data() + pos, ret * sizeof(typename STR::value_type));
  return ret;
}

size_t copy(const StringPiece& self, char* buf, size_t n, size_t pos) {
  return copyT(self, buf, n, pos);
}

template <typename STR>
size_t findT(const BasicStringPiece<STR>& self, const BasicStringPiece<STR>& s, size_t pos) {
  if (pos > self.size()) return BasicStringPiece<STR>::npos;

  typename BasicStringPiece<STR>::const_iterator result =
      std::search(self.begin() + pos, self.end(), s.begin(), s.end());
  const size_t xpos = static_cast<size_t>(result - self.begin());
  return xpos + s.size() <= self.size() ? xpos : BasicStringPiece<STR>::npos;
}

size_t find(const StringPiece& self, const StringPiece& s, size_t pos) {
  return findT(self, s, pos);
}

template <typename STR>
size_t findT(const BasicStringPiece<STR>& self, typename STR::value_type c, size_t pos) {
  if (pos >= self.size()) return BasicStringPiece<STR>::npos;

  typename BasicStringPiece<STR>::const_iterator result =
      std::find(self.begin() + pos, self.end(), c);
  return result != self.end() ? static_cast<size_t>(result - self.begin())
                              : BasicStringPiece<STR>::npos;
}

size_t find(const StringPiece& self, char c, size_t pos) { return findT(self, c, pos); }

template <typename STR>
size_t rfindT(const BasicStringPiece<STR>& self, const BasicStringPiece<STR>& s, size_t pos) {
  if (self.size() < s.size()) return BasicStringPiece<STR>::npos;

  if (s.empty()) return std::min(self.size(), pos);

  typename BasicStringPiece<STR>::const_iterator last =
      self.begin() + std::min(self.size() - s.size(), pos) + s.size();
  typename BasicStringPiece<STR>::const_iterator result =
      std::find_end(self.begin(), last, s.begin(), s.end());
  return result != last ? static_cast<size_t>(result - self.begin()) : BasicStringPiece<STR>::npos;
}

size_t rfind(const StringPiece& self, const StringPiece& s, size_t pos) {
  return rfindT(self, s, pos);
}

template <typename STR>
size_t rfindT(const BasicStringPiece<STR>& self, typename STR::value_type c, size_t pos) {
  if (self.size() == 0) return BasicStringPiece<STR>::npos;

  for (size_t i = std::min(pos, self.size() - 1);; --i) {
    if (self.data()[i] == c) return i;
    if (i == 0) break;
  }
  return BasicStringPiece<STR>::npos;
}

size_t rfind(const StringPiece& self, char c, size_t pos) { return rfindT(self, c, pos); }

// 8-bit version using lookup table.
size_t find_first_of(const StringPiece& self, const StringPiece& s, size_t pos) {
  if (self.size() == 0 || s.size() == 0) return StringPiece::npos;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1) return find(self, s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = {false};
  BuildLookupTable(s, lookup);
  for (size_t i = pos; i < self.size(); ++i) {
    if (lookup[static_cast<unsigned char>(self.data()[i])]) {
      return i;
    }
  }
  return StringPiece::npos;
}

// 8-bit version using lookup table.
size_t find_first_not_of(const StringPiece& self, const StringPiece& s, size_t pos) {
  if (self.size() == 0) return StringPiece::npos;

  if (s.size() == 0) return 0;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1) return find_first_not_of(self, s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = {false};
  BuildLookupTable(s, lookup);
  for (size_t i = pos; i < self.size(); ++i) {
    if (!lookup[static_cast<unsigned char>(self.data()[i])]) {
      return i;
    }
  }
  return StringPiece::npos;
}

template <typename STR>
size_t find_first_not_ofT(const BasicStringPiece<STR>& self, typename STR::value_type c,
                          size_t pos) {
  if (self.size() == 0) return BasicStringPiece<STR>::npos;

  for (; pos < self.size(); ++pos) {
    if (self.data()[pos] != c) {
      return pos;
    }
  }
  return BasicStringPiece<STR>::npos;
}

size_t find_first_not_of(const StringPiece& self, char c, size_t pos) {
  return find_first_not_ofT(self, c, pos);
}

// 8-bit version using lookup table.
size_t find_last_of(const StringPiece& self, const StringPiece& s, size_t pos) {
  if (self.size() == 0 || s.size() == 0) return StringPiece::npos;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1) return rfind(self, s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = {false};
  BuildLookupTable(s, lookup);
  for (size_t i = std::min(pos, self.size() - 1);; --i) {
    if (lookup[static_cast<unsigned char>(self.data()[i])]) return i;
    if (i == 0) break;
  }
  return StringPiece::npos;
}

// 8-bit version using lookup table.
size_t find_last_not_of(const StringPiece& self, const StringPiece& s, size_t pos) {
  if (self.size() == 0) return StringPiece::npos;

  size_t i = std::min(pos, self.size() - 1);
  if (s.size() == 0) return i;

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.size() == 1) return find_last_not_of(self, s.data()[0], pos);

  bool lookup[UCHAR_MAX + 1] = {false};
  BuildLookupTable(s, lookup);
  for (;; --i) {
    if (!lookup[static_cast<unsigned char>(self.data()[i])]) return i;
    if (i == 0) break;
  }
  return StringPiece::npos;
}

template <typename STR>
size_t find_last_not_ofT(const BasicStringPiece<STR>& self, typename STR::value_type c,
                         size_t pos) {
  if (self.size() == 0) return BasicStringPiece<STR>::npos;

  for (size_t i = std::min(pos, self.size() - 1);; --i) {
    if (self.data()[i] != c) return i;
    if (i == 0) break;
  }
  return BasicStringPiece<STR>::npos;
}

size_t find_last_not_of(const StringPiece& self, char c, size_t pos) {
  return find_last_not_ofT(self, c, pos);
}

template <typename STR>
BasicStringPiece<STR> substrT(const BasicStringPiece<STR>& self, size_t pos, size_t n) {
  if (pos > self.size()) pos = self.size();
  if (n > self.size() - pos) n = self.size() - pos;
  return BasicStringPiece<STR>(self.data() + pos, n);
}

StringPiece substr(const StringPiece& self, size_t pos, size_t n) { return substrT(self, pos, n); }

}  // namespace internal
}  // namespace utils
}  // namespace agora
