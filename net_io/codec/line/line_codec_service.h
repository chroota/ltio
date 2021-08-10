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

#ifndef NET_LINE_PROTO_SERVICE_H
#define NET_LINE_PROTO_SERVICE_H

#include <net_io/codec/codec_service.h>
#include "base/message_loop/message_loop.h"

namespace lt {
namespace net {

class LineCodecService : public CodecService {
public:
  LineCodecService(base::MessageLoop* loop);
  ~LineCodecService();

  void OnDataReceived(IOBuffer*) override;

  bool SendRequest(CodecMessage* request) override;

  bool SendResponse(const CodecMessage* req, CodecMessage* res) override;

private:
  bool SendMessage(CodecMessage* message);
};

}  // namespace net
}  // namespace lt
#endif
