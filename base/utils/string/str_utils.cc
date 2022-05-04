/*
 * Copyright 2021 <name of copyright holder>
 * Author: Huan.Gong <gonghuan.dev@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "str_utils.h"

#include <glog/logging.h>
#include <iostream>

namespace base {

bool StrUtil::ToInt(const std::string& s, int* out) {
  std::size_t pos{};
  try {
    *out = std::stoi(s, &pos);
    return true;
  } catch(std::invalid_argument const& ex) {
    LOG_EVERY_N(ERROR, 1000) << "conv:" << s << "fail std::invalid_argument::what(): " << ex.what();
  } catch(std::out_of_range const& ex) {
    LOG_EVERY_N(ERROR, 1000) << "conv:" << s << "fail std::out_of_range::what(): " << ex.what();
  }
  LOG(ERROR) << "unknown error, input:" << s;
  return false;
}

bool StrUtil::ToInt64(const std::string& s, int64_t* out) {
  std::size_t pos{};
  try {
    *out = std::stoll(s, &pos);
    return true;
  } catch(std::invalid_argument const& ex) {
    LOG_EVERY_N(ERROR, 1000) << "conv:" << s << "fail std::invalid_argument::what(): " << ex.what();
  } catch(std::out_of_range const& ex) {
    LOG_EVERY_N(ERROR, 1000) << "conv:" << s << "fail std::out_of_range::what(): " << ex.what();
  }
  LOG(ERROR) << "unknown error, input:" << s;
  return false;
}

void StrUtil::ToUpper(char* s) {
  for (size_t i = 0; s[i]; i++)
    s[i] = toupper(s[i]);
}

void StrUtil::ToUpper(std::string& str) {
  for (size_t i = 0; str[i]; i++)
    str[i] = toupper(str[i]);
}

void StrUtil::ToLower(char* s) {
  for (size_t i = 0; s[i]; i++)
    s[i] = tolower(s[i]);
}

void StrUtil::ToLower(std::string& str) {
  for (size_t i = 0; str[i]; i++)
    str[i] = tolower(str[i]);
}

bool StrUtil::IgnoreCaseEquals(const std::string& str1,
                               const std::string& str2) {
  return std::equal(str1.begin(), str1.end(), str2.begin(),
                    __detail__::CaseInsensitiveCmp());
}

bool StrUtil::Replace(std::string& str,
                      const std::string& from,
                      const std::string& to) {
  size_t start_pos = str.find(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

bool StrUtil::ReplaceLast(std::string& str,
                          const std::string& from,
                          const std::string& to) {
  size_t start_pos = str.rfind(from);
  if (start_pos == std::string::npos)
    return false;
  str.replace(start_pos, from.length(), to);
  return true;
}

bool StrUtil::Contains(const std::string& input, const std::string& test) {
  return std::search(input.begin(), input.end(), test.begin(), test.end()) !=
         input.end();
}

bool StrUtil::StartsWith(const std::string& str, const std::string& prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}

bool StrUtil::EndsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string& StrUtil::TrimLeft(std::string& str, const std::string& chars) {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

std::string& StrUtil::TrimRight(std::string& str, const std::string& chars) {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}
std::string& StrUtil::Trim(std::string& str, const std::string& chars) {
  return TrimLeft(TrimRight(str, chars), chars);
}

std::vector<std::string> StrUtil::Split(const std::string& str,
                                        const char delim) {
  std::vector<std::string> elems;
  std::string token;
  std::stringstream ss(str);
  while (std::getline(ss, token, delim)) {
    elems.push_back(token);
  }
  return elems;
}

std::vector<std::string> StrUtil::Split(const std::string& text,
                                        const std::string& delims,
                                        bool ignore_empty) {
  std::vector<std::string> tokens;
  std::size_t current, previous = 0;
  current = text.find(delims);
  while (current != std::string::npos) {
    if (current > previous) {
      tokens.push_back(text.substr(previous, current - previous));
    } else if (!ignore_empty) {
      tokens.push_back(text.substr(previous, current - previous));
    }
    previous = current + 1;
    current = text.find(delims, previous);
  }
  if (current > previous) {
    tokens.push_back(text.substr(previous, current - previous));
  } else if (!ignore_empty) {
    tokens.push_back(text.substr(previous, current - previous));
  }
  return tokens;
}

template <class string_type>
inline typename string_type::value_type* WriteIntoT(string_type* str,
                                                    size_t length_with_null) {
  DCHECK_GE(length_with_null, 1u);
  str->reserve(length_with_null);
  str->resize(length_with_null - 1);
  return &((*str)[0]);
}

char* AsWriteInto(std::string* str, size_t length_with_null) {
  return WriteIntoT(str, length_with_null);
}

}  // namespace base
