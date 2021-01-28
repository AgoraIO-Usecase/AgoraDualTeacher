//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//
#pragma once
//
// A string-like object that points to a sized piece of memory.
//
// You can use StringPiece as a function or method parameter.  A StringPiece
// parameter can receive a double-quoted string literal argument, a "const
// char*" argument, a string argument, or a StringPiece argument with no data
// copying.  Systematic use of StringPiece for arguments reduces data
// copies and strlen() calls.
//
// Prefer passing StringPieces by value:
//   void MyFunction(StringPiece arg);
// If circumstances require, you may also pass by const reference:
//   void MyFunction(const StringPiece& arg);  // not preferred
// Both of these have the same lifetime semantics.  Passing by value
// generates slightly smaller code.  For more discussion, Googlers can see
// the thread go/stringpiecebyvalue on c-users.

#include <stddef.h>

#include <iosfwd>
#include <string>

#include "utils/strings/char_traits.h"
#include "utils/strings/string_piece_forward.h"

namespace agora {
namespace utils {

// internal --------------------------------------------------------------------

// Many of the StringPiece functions use different implementations for the
// 8-bit and 16-bit versions, and we don't want lots of template expansions in
// this (very common) header that will slow down compilation.
//
// So here we define overloaded functions called by the StringPiece template.
// For those that share an implementation, the two versions will expand to a
// template internal to the .cc file.
namespace internal {

void CopyToString(const StringPiece& self, std::string* target);

void AppendToString(const StringPiece& self, std::string* target);

size_t copy(const StringPiece& self, char* buf, size_t n, size_t pos);

size_t find(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find(const StringPiece& self, char c, size_t pos);

size_t rfind(const StringPiece& self, const StringPiece& s, size_t pos);
size_t rfind(const StringPiece& self, char c, size_t pos);

size_t find_first_of(const StringPiece& self, const StringPiece& s, size_t pos);

size_t find_first_not_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_first_not_of(const StringPiece& self, char c, size_t pos);

size_t find_last_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_last_of(const StringPiece& self, char c, size_t pos);

size_t find_last_not_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_last_not_of(const StringPiece& self, char c, size_t pos);

StringPiece substr(const StringPiece& self, size_t pos, size_t n);

}  // namespace internal

// BasicStringPiece ------------------------------------------------------------

// Defines the types, methods, operators, and data members common to both
// StringPiece and StringPiece16. Do not refer to this class directly, but
// rather to BasicStringPiece, StringPiece, or StringPiece16.
//
// This is templatized by string class type rather than character type, so
// BasicStringPiece<std::string>
template <typename STRING_TYPE>
class BasicStringPiece {
 public:
  // Standard STL container boilerplate.
  typedef size_t size_type;
  typedef typename STRING_TYPE::value_type value_type;
  typedef const value_type* pointer;
  typedef const value_type& reference;
  typedef const value_type& const_reference;
  typedef ptrdiff_t difference_type;
  typedef const value_type* const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  static const size_type npos;

 public:
  // We provide non-explicit singleton constructors so users can pass
  // in a "const char*" or a "string" wherever a "StringPiece" is
  // expected (likewise for char16, string16, StringPiece16).
  constexpr BasicStringPiece() : ptr_(NULL), length_(0) {}
  // TODO(dcheng): Construction from nullptr is not allowed for
  // std::basic_string_view, so remove the special handling for it.
  // Note: This doesn't just use STRING_TYPE::traits_type::length(), since that
  // isn't constexpr until C++17.
  constexpr BasicStringPiece(const value_type* str)
      : ptr_(str), length_(!str ? 0 : CharTraits<value_type>::length(str)) {}
  BasicStringPiece(const STRING_TYPE& str) : ptr_(str.data()), length_(str.size()) {}
  constexpr BasicStringPiece(const value_type* offset, size_type len)
      : ptr_(offset), length_(len) {}
  BasicStringPiece(const typename STRING_TYPE::const_iterator& begin,
                   const typename STRING_TYPE::const_iterator& end) {
    length_ = static_cast<size_t>(std::distance(begin, end));

    // The length test before assignment is to avoid dereferencing an iterator
    // that may point to the end() of a string.
    ptr_ = length_ > 0 ? &*begin : nullptr;
  }

  // data() may return a pointer to a buffer with embedded NULs, and the
  // returned buffer may or may not be null terminated.  Therefore it is
  // typically a mistake to pass data() to a routine that expects a NUL
  // terminated string.
  constexpr const value_type* data() const { return ptr_; }
  constexpr size_type size() const { return length_; }
  constexpr size_type length() const { return length_; }
  constexpr bool empty() const { return length_ == 0; }

  void clear() {
    ptr_ = NULL;
    length_ = 0;
  }
  void set(const value_type* data, size_type len) {
    ptr_ = data;
    length_ = len;
  }
  void set(const value_type* str) {
    ptr_ = str;
    length_ = str ? STRING_TYPE::traits_type::length(str) : 0;
  }

  constexpr value_type operator[](size_type i) const { return ptr_[i]; }

  value_type front() const { return ptr_[0]; }

  value_type back() const { return ptr_[length_ - 1]; }

  constexpr void remove_prefix(size_type n) {
    ptr_ += n;
    length_ -= n;
  }

  constexpr void remove_suffix(size_type n) { length_ -= n; }

  constexpr int compare(BasicStringPiece x) const noexcept {
    int r =
        CharTraits<value_type>::compare(ptr_, x.ptr_, (length_ < x.length_ ? length_ : x.length_));
    if (r == 0) {
      if (length_ < x.length_)
        r = -1;
      else if (length_ > x.length_)
        r = +1;
    }
    return r;
  }

  // This is the style of conversion preferred by std::string_view in C++17.
  explicit operator STRING_TYPE() const { return as_string(); }

  STRING_TYPE as_string() const {
    // std::string doesn't like to take a NULL pointer even with a 0 size.
    return empty() ? STRING_TYPE() : STRING_TYPE(data(), size());
  }

  const_iterator begin() const { return ptr_; }
  const_iterator end() const { return ptr_ + length_; }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(ptr_ + length_); }
  const_reverse_iterator rend() const { return const_reverse_iterator(ptr_); }

  size_type max_size() const { return length_; }
  size_type capacity() const { return length_; }

  // Sets the value of the given string target type to be the current string.
  // This saves a temporary over doing |a = b.as_string()|
  void CopyToString(STRING_TYPE* target) const { internal::CopyToString(*this, target); }

  void AppendToString(STRING_TYPE* target) const { internal::AppendToString(*this, target); }

  size_type copy(value_type* buf, size_type n, size_type pos = 0) const {
    return internal::copy(*this, buf, n, pos);
  }

  // Does "this" start with "x"
  constexpr bool starts_with(BasicStringPiece x) const noexcept {
    return ((this->length_ >= x.length_) &&
            (CharTraits<value_type>::compare(this->ptr_, x.ptr_, x.length_) == 0));
  }

  // Does "this" end with "x"
  constexpr bool ends_with(BasicStringPiece x) const noexcept {
    return ((this->length_ >= x.length_) &&
            (CharTraits<value_type>::compare(this->ptr_ + (this->length_ - x.length_), x.ptr_,
                                             x.length_) == 0));
  }

  // find: Search for a character or substring at a given offset.
  size_type find(const BasicStringPiece<STRING_TYPE>& s, size_type pos = 0) const {
    return internal::find(*this, s, pos);
  }
  size_type find(value_type c, size_type pos = 0) const { return internal::find(*this, c, pos); }

  // rfind: Reverse find.
  size_type rfind(const BasicStringPiece& s, size_type pos = BasicStringPiece::npos) const {
    return internal::rfind(*this, s, pos);
  }
  size_type rfind(value_type c, size_type pos = BasicStringPiece::npos) const {
    return internal::rfind(*this, c, pos);
  }

  // find_first_of: Find the first occurence of one of a set of characters.
  size_type find_first_of(const BasicStringPiece& s, size_type pos = 0) const {
    return internal::find_first_of(*this, s, pos);
  }
  size_type find_first_of(value_type c, size_type pos = 0) const { return find(c, pos); }

  // find_first_not_of: Find the first occurence not of a set of characters.
  size_type find_first_not_of(const BasicStringPiece& s, size_type pos = 0) const {
    return internal::find_first_not_of(*this, s, pos);
  }
  size_type find_first_not_of(value_type c, size_type pos = 0) const {
    return internal::find_first_not_of(*this, c, pos);
  }

  // find_last_of: Find the last occurence of one of a set of characters.
  size_type find_last_of(const BasicStringPiece& s, size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_of(*this, s, pos);
  }
  size_type find_last_of(value_type c, size_type pos = BasicStringPiece::npos) const {
    return rfind(c, pos);
  }

  // find_last_not_of: Find the last occurence not of a set of characters.
  size_type find_last_not_of(const BasicStringPiece& s,
                             size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_not_of(*this, s, pos);
  }
  size_type find_last_not_of(value_type c, size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_not_of(*this, c, pos);
  }

  // substr.
  BasicStringPiece substr(size_type pos, size_type n = BasicStringPiece::npos) const {
    return internal::substr(*this, pos, n);
  }

 protected:
  const value_type* ptr_;
  size_type length_;
};

template <typename STRING_TYPE>
const typename BasicStringPiece<STRING_TYPE>::size_type BasicStringPiece<STRING_TYPE>::npos =
    typename BasicStringPiece<STRING_TYPE>::size_type(-1);

// MSVC doesn't like complex extern templates and DLLs.
#if !defined(COMPILER_MSVC)
extern template class BasicStringPiece<std::string>;
#endif

// StingPiece operators --------------------------------------------------------

bool operator==(const StringPiece& x, const StringPiece& y);

inline bool operator!=(const StringPiece& x, const StringPiece& y) { return !(x == y); }

inline bool operator<(const StringPiece& x, const StringPiece& y) {
  const int r = CharTraits<StringPiece::value_type>::compare(
      x.data(), y.data(), (x.size() < y.size() ? x.size() : y.size()));
  return ((r < 0) || ((r == 0) && (x.size() < y.size())));
}

inline bool operator>(const StringPiece& x, const StringPiece& y) { return y < x; }

inline bool operator<=(const StringPiece& x, const StringPiece& y) { return !(x > y); }

inline bool operator>=(const StringPiece& x, const StringPiece& y) { return !(x < y); }

std::ostream& operator<<(std::ostream& o, const StringPiece& piece);

// Hashing ---------------------------------------------------------------------

// We provide appropriate hash functions so StringPiece and StringPiece16 can
// be used as keys in hash sets and maps.

// This hash function is copied from base/strings/string16.h. We don't use the
// ones already defined for string and string16 directly because it would
// require the string constructors to be called, which we don't want.
#define HASH_STRING_PIECE(StringPieceType, string_piece)                                       \
  std::size_t result = 0;                                                                      \
  for (StringPieceType::const_iterator i = string_piece.begin(); i != string_piece.end(); ++i) \
    result = (result * 131) + *i;                                                              \
  return result;

struct StringPieceHash {
  std::size_t operator()(const StringPiece& sp) const { HASH_STRING_PIECE(StringPiece, sp); }
};

struct WStringPieceHash {
  std::size_t operator()(const WStringPiece& wsp) const { HASH_STRING_PIECE(WStringPiece, wsp); }
};

}  // namespace utils
}  // namespace agora
