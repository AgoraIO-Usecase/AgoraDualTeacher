//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//
#pragma once

#include <string>
#include <utility>
#include <vector>

#include "utils/strings/string_piece.h"

namespace agora {
namespace utils {

enum WhitespaceHandling {
  KEEP_WHITESPACE,
  TRIM_WHITESPACE,
};

enum SplitResult {
  // Strictly return all results.
  //
  // If the input is ",," and the separator is ',' this will return a
  // vector of three empty strings.
  SPLIT_WANT_ALL,

  // Only nonempty results will be added to the results. Multiple separators
  // will be coalesced. Separators at the beginning and end of the input will
  // be ignored. With TRIM_WHITESPACE, whitespace-only results will be dropped.
  //
  // If the input is ",," and the separator is ',', this will return an empty
  // vector.
  SPLIT_WANT_NONEMPTY,
};

// Split the given string on ANY of the given separators, returning copies of
// the result.
//
// To split on either commas or semicolons, keeping all whitespace:
//
//   std::vector<std::string> tokens = agora::utils::SplitString(
//       input, ",;", agora::utils::KEEP_WHITESPACE, agora::utils::SPLIT_WANT_ALL);
std::vector<std::string> SplitString(StringPiece input, StringPiece separators,
                                     WhitespaceHandling whitespace, SplitResult result_type);

// Like SplitString above except it returns a vector of StringPieces which
// reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// To iterate through all whitespace-separated tokens in an input string:
//
//   for (const auto& cur :
//        agora::utils::SplitStringPiece(input, agora::utils::kWhitespaceASCII,
//                               agora::utils::KEEP_WHITESPACE,
//                               agora::utils::SPLIT_WANT_NONEMPTY)) {
//     ...
std::vector<StringPiece> SplitStringPiece(StringPiece input, StringPiece separators,
                                          WhitespaceHandling whitespace, SplitResult result_type);

using StringPairs = std::vector<std::pair<std::string, std::string>>;

// Splits |line| into key value pairs according to the given delimiters and
// removes whitespace leading each key and trailing each value. Returns true
// only if each pair has a non-empty key and value. |key_value_pairs| will
// include ("","") pairs for entries without |key_value_delimiter|.
bool SplitStringIntoKeyValuePairs(StringPiece input, char key_value_delimiter,
                                  char key_value_pair_delimiter, StringPairs* key_value_pairs);

// Similar to SplitString, but use a substring delimiter instead of a list of
// characters that are all possible delimiters.
std::vector<std::string> SplitStringUsingSubstr(StringPiece input, StringPiece delimiter,
                                                WhitespaceHandling whitespace,
                                                SplitResult result_type);

// Like SplitStringUsingSubstr above except it returns a vector of StringPieces
// which reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// To iterate through all newline-separated tokens in an input string:
//
//   for (const auto& cur :
//        agora::utils::SplitStringUsingSubstr(input, "\r\n",
//                                     agora::utils::KEEP_WHITESPACE,
//                                     agora::utils::SPLIT_WANT_NONEMPTY)) {
//     ...
std::vector<StringPiece> SplitStringPieceUsingSubstr(StringPiece input, StringPiece delimiter,
                                                     WhitespaceHandling whitespace,
                                                     SplitResult result_type);

}  // namespace utils
}  // namespace agora
