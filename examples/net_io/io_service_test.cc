#include "base/message_loop/message_loop.h"
#include "glog/logging.h"
#include "net_io/tcp_channel.h"
#include "net_io/io_service.h"
#include "net_io/codec/codec_service.h"
#include "net_io/codec/codec_factory.h"
#include "base/message_loop/linux_signal.h"

namespace lt {
namespace net {

class TcpCodecService : public CodecService {
public:
  TcpCodecService() :
    CodecService(nullptr) {
  }
  ~TcpCodecService() {
  }
  bool SendRequest(CodecMessage* message) override {
    return true;
  };
  bool SendResponse(const CodecMessage* req, CodecMessage* res) override {
    return true;
  };

  void OnDataFinishSend(const SocketChannel* channel) {
    LOG(INFO) << " channel write data finished";
  }

  void OnDataReceived(IOBuffer *buffer) override {
    LOG(INFO) << " channel recieve data" << buffer->CanReadSize();
    int32_t size = Channel()->Send(buffer->GetRead(), buffer->CanReadSize());
    if (size > 0) {
      buffer->Consume(size);
    }
    //channel->ShutdownChannel();
  }
};

class Srv : public IOServiceDelegate {
public:
  Srv() {
    tcp_protoservice_.reset(new TcpCodecService);
    InitWorkLoop();
    net::IPAddress ip;
    ip.AssignFromIPLiteral("127.0.0.1");
    net::IPEndPoint ep(ip, 5005);
    ioservice_.reset(new IOService(ep, "tcp", &acceptor_loop_, this));
  }

  ~Srv() {
  }

  void InitWorkLoop() {
    iowork_loop_.SetLoopName("iowork_loop");
    iowork_loop_.Start();
    acceptor_loop_.SetLoopName("acceptor_loop");
    acceptor_loop_.Start();
  }

  void Start() {
    RegisterExitSignal();

    CodecFactory::RegisterCreator(
      "tcp", [](base::MessageLoop*)->CodecServicePtr {
      return CodecServicePtr(new TcpCodecService);
    });
    acceptor_loop_.PostTask(FROM_HERE, &IOService::Start, ioservice_);
    acceptor_loop_.WaitLoopEnd();
  }

  void IOServiceStoped(IOService* ioservice) {
    LOG(INFO) << " IOService stop ok";
    CHECK(ioservice_.get() == ioservice);
    ioservice_.reset();
    acceptor_loop_.QuitLoop();
  };

  base::MessageLoop* GetNextIOWorkLoop() {
    return &iowork_loop_;
  }
  void IncreaseChannelCount() {
    LOG(INFO) << " a new channel comming";
  }
  void DecreaseChannelCount() {
    LOG(INFO) << " a channel going gone";
  }
  bool CanCreateNewChannel() {
    return true;
  }
  CodecService* GetProtocolService(const std::string protocol) {
    return tcp_protoservice_.get();
  }

  void OnRequestMessage(const RefCodecMessage& request) {

  };
private:
  void RegisterExitSignal() {
    //CHECK(acceptor_loop_.IsInLoopThread());
    base::Signal::signal(10, std::bind(&Srv::ExitSignalHandle, this));
  }
  void ExitSignalHandle() {
    LOG(INFO) << " ExitSignalHandle quit AcceptorLoop";
    acceptor_loop_.PostTask(
      NewClosure(std::bind(&IOService::Stop, ioservice_.get())));
  }
  base::MessageLoop iowork_loop_;
  base::MessageLoop acceptor_loop_;

  RefIOService ioservice_;
  CodecServicePtr tcp_protoservice_;
};

}}

using namespace lt;

int main(int argc, char** argv) {
  net::Srv s;
  s.Start();
  //LOG(INFO) << " main is going to end";
  return 0;
}
