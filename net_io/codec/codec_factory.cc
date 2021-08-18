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

#include "codec_factory.h"
#include "base/message_loop/message_loop.h"
#include "http/http_codec_service.h"
#include "line/line_codec_service.h"
#include "raw/raw_codec_service.h"
#include "redis/resp_codec_service.h"
#include "websocket/ws_codec_service.h"

#include <base/memory/lazy_instance.h>

namespace lt {
namespace net {

// static
CodecFactory& Instance() {
  static base::LazyInstance<CodecFactory> instance = LAZY_INSTANCE_INIT;
  return instance.get();
}

CodecFactory::CodecFactory() {
  InitInnerDefault();
}

// static
RefCodecService CodecFactory::NewServerService(const std::string& proto,
                                               base::MessageLoop* loop) {
  return Instance().CreateProtocolService(proto, loop, true);
}
// static
RefCodecService CodecFactory::NewClientService(const std::string& proto,
                                               base::MessageLoop* loop) {
  return Instance().CreateProtocolService(proto, loop, false);
}

RefCodecService CodecFactory::CreateProtocolService(const std::string& proto,
                                                    base::MessageLoop* loop,
                                                    bool server_service) {
  auto iter = Instance().creators_.find(proto);
  if (iter != Instance().creators_.end() && iter->second) {
    auto codec = iter->second(loop);
    codec->SetProtocol(proto);
    codec->SetIsServerSide(server_service);
    return codec;
  }
  LOG(ERROR) << __FUNCTION__ << " Protocol:" << proto << " Not Supported";
  static RefCodecService _null;
  return _null;
}

// not thread safe,
void CodecFactory::RegisterCreator(const std::string proto,
                                   ProtoserviceCreator creator) {
  CodecFactory& factory = Instance();
  factory.creators_[proto] = creator;
}

bool CodecFactory::HasCreator(const std::string& proto) {
  CodecFactory& factory = Instance();
  return factory.creators_.find(proto) != factory.creators_.end();
}

void CodecFactory::InitInnerDefault() {
  creators_.insert(
      std::make_pair("line", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<LineCodecService> service(new LineCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("http", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<HttpCodecService> service(new HttpCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("https", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<HttpCodecService> service(new HttpCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("raw", [](base::MessageLoop* loop) -> RefCodecService {
        auto service = std::make_shared<RawCodecService<RawMessage>>(loop);
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("rawSSL", [](base::MessageLoop* loop) -> RefCodecService {
        auto service = std::make_shared<RawCodecService<RawMessage>>(loop);
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("redis", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<RespCodecService> service(new RespCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("ws", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<CodecService> service(new WSCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
  creators_.insert(
      std::make_pair("wss", [](base::MessageLoop* loop) -> RefCodecService {
        std::shared_ptr<CodecService> service(new WSCodecService(loop));
        return std::static_pointer_cast<CodecService>(service);
      }));
}

}  // namespace net
}  // namespace lt
