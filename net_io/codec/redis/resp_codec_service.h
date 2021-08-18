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

#ifndef _NET_PROTOCOL_RESP_SERVICE_H_H
#define _NET_PROTOCOL_RESP_SERVICE_H_H

#include <net_io/codec/codec_service.h>
#include "redis_response.h"

namespace lt {
namespace net {

class RedisResponse;
/*
 * A RESP protocol client side codec service
 * Note: not support as service side service
 *
 * - 支持客户端对db/auth等初始化操作
 * - 支持以Redis Ping命令作为心跳请求来与服务端保持活跃确认
 * */
class RespCodecService : public CodecService {
public:
  typedef enum _ {
    kWaitNone = 0x0,
    kWaitAuth = 0x01,
    kWaitSelectDB = 0x01 << 1,
  } InitWaitFlags;

  RespCodecService(base::MessageLoop* loop);

  ~RespCodecService();

  void StartProtocolService() override;

  void OnDataReceived(IOBuffer*) override;

  bool SendRequest(CodecMessage* message) override;

  bool SendResponse(const CodecMessage* req, CodecMessage* res) override;

  bool KeepHeartBeat() override { return true; }

  const RefCodecMessage NewHeartbeat() override;

private:
  void HandleInitResponse(RedisResponse* response);
  uint8_t init_wait_res_flags_ = 0;
  uint32_t next_incoming_count_ = 0;
  RefRedisResponse current_response;  // = std::make_shared<RedisResponse>();
  resp::decoder decoder_;
};

}  // namespace net
}  // namespace lt
#endif
