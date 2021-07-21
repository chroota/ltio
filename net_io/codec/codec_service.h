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

#ifndef _NET_PROTOCOL_SERVICE_H_H
#define _NET_PROTOCOL_SERVICE_H_H

#include "codec_message.h"
#include "net_io/base/ip_endpoint.h"
#include "net_io/channel.h"
#include "net_io/net_callback.h"
#include "net_io/url_utils.h"

namespace base {
class MessageLoop;
}

namespace lt {
namespace net {

/* a stateless encoder/decoder and
 * transfer the ProtoMessage to real Handler */
class CodecService : public EnableShared(CodecService),
                     public SocketChannel::Reciever {
public:
  class Delegate {
  public:
    virtual void OnCodecMessage(const RefCodecMessage& message) = 0;

    virtual void OnProtocolServiceReady(const RefCodecService& service){};

    virtual void OnProtocolServiceGone(const RefCodecService& service) = 0;

    // protocol upgrade
    virtual bool UpgradeProtocol(const RefCodecService& service,
                                 const RefCodecMessage& upgrade_req) {
      return false;
    }

    // for client side
    virtual const url::RemoteInfo* GetRemoteInfo() const { return NULL; };
  };

public:
  CodecService(base::MessageLoop* loop);

  virtual ~CodecService();

  void SetDelegate(Delegate* d);

  void SetProtocol(const std::string& protocol) { protocol_ = protocol; };

  void BindSocket(SocketChannelPtr&& channel);

  virtual void StartProtocolService();

  base::EventPump* Pump() const;

  base::MessageLoop* IOLoop() const { return binded_loop_; }

  SocketChannel* Channel() { return channel_.get(); };

  void CloseService(bool block_callback = false);

  bool IsConnected() const;

  virtual bool FromUpgrade(RefCodecMessage& req) { return false; };

  virtual void BeforeCloseService(){};

  virtual void AfterChannelClosed(){};

  /* feature indentify*/
  // async clients request
  virtual bool KeepSequence() { return true; };

  virtual bool KeepHeartBeat() { return false; }

  virtual bool SendRequest(CodecMessage* message) = 0;

  virtual bool SendResponse(const CodecMessage* req, CodecMessage* res) = 0;

  virtual const RefCodecMessage NewHeartbeat() { return NULL; }

  virtual const RefCodecMessage NewResponse(const CodecMessage*) {
    return NULL;
  }

  const std::string& protocol() const { return protocol_; };

  void SetIsServerSide(bool server_side);

  virtual bool UseSSLChannel() const { return false; };

  bool IsServerSide() const override { return server_side_; }

  inline MessageType InComingType() const {
    return server_side_ ? MessageType::kRequest : MessageType::kResponse;
  }

protected:
  // override this do initializing for client side, like set db, auth etc
  virtual void OnChannelReady(const SocketChannel*) override;

  void OnChannelClosed(const SocketChannel*) override;

  bool server_side_;

  std::string protocol_;

  SocketChannelPtr channel_;

  Delegate* delegate_ = nullptr;

  base::MessageLoop* binded_loop_ = nullptr;
  DISALLOW_COPY_AND_ASSIGN(CodecService);
};

}  // namespace net
}  // namespace lt
#endif
