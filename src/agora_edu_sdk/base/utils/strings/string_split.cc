//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include "utils/strings/string_split.h"

#include <stddef.h>

#include "utils/strings/string_util.h"

namespace agora {
namespace utils {

namespace {

// PieceToOutputType converts a StringPiece as needed to a given output type,
// which is either the same type of StringPiece (a NOP) or the corresponding
// non-piece string type.
//
// The default converter is a NOP, it works when the OutputType is the
// correct StringPiece.
template <typename Str, typename OutputType>
OutputType PieceToOutputType(BasicStringPiece<Str> piece) {
  return piece;
}
template <>  // Convert StringPiece to std::string
std::string PieceToOutputType<std::string, std::string>(StringPiece piece) {
  return piece.as_string();
}

// Returns either the ASCII or UTF-16 whitespace.
template <typename Str>
BasicStringPiece<Str> WhitespaceForType();

template <>
StringPiece WhitespaceForType<std::string>() {
  return kWhitespaceASCII;
}

// Optimize the single-character case to call find() on the string instead,
// since this is the common case and can be made faster. This could have been
// done with template specialization too, but would have been less clear.
//
// There is no corresponding FindFirstNotOf because StringPiece already
// implements these different versions that do the optimized searching.
size_t FindFirstOf(StringPiece piece, char c, size_t pos) { return piece.find(c, pos); }
size_t FindFirstOf(StringPiece piece, StringPiece one_of, size_t pos) {
  return piece.find_first_of(one_of, pos);
}

// General string splitter template. Can take 8- or 16-bit input, can produce
// the corresponding string or StringPiece output, and can take single- or
// multiple-character delimiters.
//
// DelimiterType is either a character (Str::value_type) or a string piece of
// multiple characters (BasicStringPiece<Str>). StringPiece has a version of
// find for both of these cases, and the single-character version is the most
// common and can be implemented faster, which is why this is a template.
template <typename Str, typename OutputStringType, typename DelimiterType>
static std::vector<OutputStringType> SplitStringT(BasicStringPiece<Str> str,
                                                  DelimiterType delimiter,
                                                  WhitespaceHandling whitespace,
                                                  SplitResult result_type) {
  std::vector<OutputStringType> result;
  if (str.empty()) return result;

  size_t start = 0;
  while (start != Str::npos) {
    size_t end = FindFirstOf(str, delimiter, start);

    BasicStringPiece<Str> piece;
    if (end == Str::npos) {
      piece = str.substr(start);
      start = Str::npos;
    } else {
      piece = str.substr(start, end - start);
      start = end + 1;
    }

    if (whitespace == TRIM_WHITESPACE)
      piece = TrimString(piece, WhitespaceForType<Str>(), TRIM_ALL);

    if (result_type == SPLIT_WANT_ALL || !piece.empty())
      result.push_back(PieceToOutputType<Str, OutputStringType>(piece));
  }
  return result;
}

bool AppendStringKeyValue(StringPiece input, char delimiter, StringPairs* result) {
  // Always append a new item regardless of success (it might be empty). The
  // below code will copy the strings directly into the result pair.
  result->resize(result->size() + 1);
  auto& result_pair = result->back();

  // Find the delimiter.
  size_t end_key_pos = input.find_first_of(delimiter);
  if (end_key_pos == std::string::npos) {
    return false;  // No delimiter.
  }
  input.substr(0, end_key_pos).CopyToString(&result_pair.first);

  // Find the value string.
  StringPiece remains = input.substr(end_key_pos, input.size() - end_key_pos);
  size_t begin_value_pos = remains.find_first_not_of(delimiter);
  if (begin_value_pos == StringPiece::npos) {
    return false;  // No value.
  }
  remains.substr(begin_value_pos, remains.size() - begin_value_pos)
      .CopyToString(&result_pair.second);

  return true;
}

template <typename Str, typename OutputStringType>
void SplitStringUsingSubstrT(BasicStringPiece<Str> input, BasicStringPiece<Str> delimiter,
                             WhitespaceHandling whitespace, SplitResult result_type,
                             std::vector<OutputStringType>* result) {
  using Piece = BasicStringPiece<Str>;
  using size_type = typename Piece::size_type;

  result->clear();
  for (size_type begin_index = 0, end_index = 0; end_index != Piece::npos;
       begin_index = end_index + delimiter.size()) {
    end_index = input.find(delimiter, begin_index);
    Piece term = end_index == Piece::npos ? input.substr(begin_index)
                                          : input.substr(begin_index, end_index - begin_index);

    if (whitespace == TRIM_WHITESPACE) term = TrimString(term, WhitespaceForType<Str>(), TRIM_ALL);

    if (result_type == SPLIT_WANT_ALL || !term.empty())
      result->push_back(PieceToOutputType<Str, OutputStringType>(term));
  }
}

}  // namespace

std::vector<std::string> SplitString(StringPiece input, StringPiece separators,
                                     WhitespaceHandling whitespace, SplitResult result_type) {
  if (separators.size() == 1) {
    return SplitStringT<std::string, std::string, char>(input, separators[0], whitespace,
                                                        result_type);
  }
  return SplitStringT<std::string, std::string, StringPiece>(input, separators, whitespace,
                                                             result_type);
}

std::vector<StringPiece> SplitStringPiece(StringPiece input, StringPiece separators,
                                          WhitespaceHandling whitespace, SplitResult result_type) {
  if (separators.size() == 1) {
    return SplitStringT<std::string, StringPiece, char>(input, separators[0], whitespace,
                                                        result_type);
  }
  return SplitStringT<std::string, StringPiece, StringPiece>(input, separators, whitespace,
                                                             result_type);
}

bool SplitStringIntoKeyValuePairs(StringPiece input, char key_value_delimiter,
                                  char key_value_pair_delimiter, StringPairs* key_value_pairs) {
  key_value_pairs->clear();

  std::vector<StringPiece> pairs = SplitStringPiece(input, std::string(1, key_value_pair_delimiter),
                                                    TRIM_WHITESPACE, SPLIT_WANT_NONEMPTY);
  key_value_pairs->reserve(pairs.size());

  bool success = true;
  for (const StringPiece& pair : pairs) {
    if (!AppendStringKeyValue(pair, key_value_delimiter, key_value_pairs)) {
      // Don't return here, to allow for pairs without associated
      // value or key; just record that the split failed.
      success = false;
    }
  }
  return success;
}

std::vector<std::string> SplitStringUsingSubstr(StringPiece input, StringPiece delimiter,
                                                WhitespaceHandling whitespace,
                                                SplitResult result_type) {
  std::vector<std::string> result;
  SplitStringUsingSubstrT(input, delimiter, whitespace, result_type, &result);
  return result;
}

std::vector<StringPiece> SplitStringPieceUsingSubstr(StringPiece input, StringPiece delimiter,
                                                     WhitespaceHandling whitespace,
                                                     SplitResult result_type) {
  std::vector<StringPiece> result;
  SplitStringUsingSubstrT(input, delimiter, whitespace, result_type, &result);
  return result;
}

}  // namespace utils
}  // namespace agora
