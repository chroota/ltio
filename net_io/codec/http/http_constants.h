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

#ifndef _NET_HTTP_CONSTANTS_H_H
#define _NET_HTTP_CONSTANTS_H_H

#include <string>

#include "base/string/string_view.h"

namespace lt {
namespace net {

struct HttpConstant {
  static const std::string kRootPath;

  static const std::string kCRCN;
  static const std::string kHost;
  static const std::string kBlankSpace;
  static const std::string kConnection;
  static const std::string kContentType;
  static const std::string kContentLength;
  static const std::string kBadRequest;
  static const std::string kContentEncoding;
  static const std::string kAcceptEncoding;

  static const std::string kHeaderClose;
  static const std::string kHeaderKeepalive;
  static const std::string kHeaderDefaultContentType;
  static const std::string kHeaderGzipEncoding;
  static const std::string kHeaderSupportedEncoding;
};

const std::string& http_status_desc(int code);
const std::string& http_resp_head_line(int code, uint8_t subversion);

}  // namespace net
}  // namespace lt
#endif
