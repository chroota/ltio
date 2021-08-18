#ifndef _LT_COMPONENT_TCP_CHANNEL_SSL_H_
#define _LT_COMPONENT_TCP_CHANNEL_SSL_H_

#include <functional>
#include <memory>
#include "base/message_loop/event_pump.h"
#include "channel.h"
#include "codec/codec_message.h"
#include "net_callback.h"
#include "base/crypto/lt_ssl.h"

namespace lt {
namespace net {

class TCPSSLChannel;
using TCPSSLChannelPtr = std::unique_ptr<TCPSSLChannel>;

class TCPSSLChannel : public SocketChannel {
public:
  static TCPSSLChannelPtr Create(int socket_fd,
                                 const IPEndPoint& local,
                                 const IPEndPoint& peer);

  ~TCPSSLChannel();

  void InitSSL(SSLImpl* ssl);

  void StartChannel() override;

  int32_t Send(const char* data, const int32_t len) override;

protected:
  TCPSSLChannel(int socket_fd,
                const IPEndPoint& loc,
                const IPEndPoint& peer);

  // write as much as data into socket
  bool TryFlush() override;

  // return true when success, false when failed
  bool DoHandshake(base::FdEvent* event);

  bool HandleWrite(base::FdEvent* event);

  bool HandleRead(base::FdEvent* event);

  void HandleEvent(base::FdEvent* fdev) override;
private:
  bool server_ = true;

#ifdef LTIO_WITH_OPENSSL
  SSLImpl* ssl_;
#endif

  DISALLOW_COPY_AND_ASSIGN(TCPSSLChannel);
};

}  // namespace net
}  // namespace lt

#endif
