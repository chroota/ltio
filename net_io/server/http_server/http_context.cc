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

#include <glog/logging.h>
#include "base/message_loop/message_loop.h"
#include "net_io/codec/codec_service.h"

#include "http_context.h"
#include "net_io/codec/http/h2/h2_codec_service.h"

namespace lt {
namespace net {

// static
RefHttpRequestCtx HttpRequestCtx::New(const RefCodecMessage& req) {
  return RefHttpRequestCtx(new HttpRequestCtx(req));
}

HttpRequestCtx::HttpRequestCtx(const RefCodecMessage& request)
  : request_(request) {
  io_loop_ = request->GetIOCtx().io_loop;
}

void HttpRequestCtx::File(const std::string& path, uint16_t code) {
  if (did_reply_)
    return;
  CHECK(false);
}

void HttpRequestCtx::Json(const std::string& json, uint16_t code) {
  if (did_reply_)
    return;

  RefHttpResponse response = HttpResponse::CreateWithCode(code);
  response->InsertHeader("Content-Type", "application/json;utf-8");
  response->MutableBody() = json;

  return Response(response);
}

void HttpRequestCtx::String(const char* content, uint16_t code) {
  if (did_reply_)
    return;

  RefHttpResponse response = HttpResponse::CreateWithCode(code);
  response->MutableBody().append(content);

  return Response(response);
}

void HttpRequestCtx::String(const std::string& content, uint16_t code) {
  if (did_reply_)
    return;

  RefHttpResponse response = HttpResponse::CreateWithCode(code);
  response->MutableBody() = std::move(content);

  return Response(response);
}

void HttpRequestCtx::Response(RefHttpResponse& response) {
  did_reply_ = true;

  request_->SetResponse(response);

  const HttpRequest* request = Request();
  auto service = request->GetIOCtx().codec.lock();
  if (!service) {
    LOG(ERROR) << __FUNCTION__ << " Connection Has Broken";
    return;
  }

  bool keep_alive = request->IsKeepAlive();
  response->SetKeepAlive(keep_alive);

  if (!io_loop_->IsInLoopThread()) {
    auto req = request_;

    auto functor = [=]() {
      bool success = service->SendResponse(req.get(), response.get());
      if (!keep_alive || !success) {
        service->CloseService();
      }
    };
    io_loop_->PostTask(NewClosure(std::move(functor)));
    return;
  }

  bool success = service->SendResponse(request_.get(), response.get());
  if (!keep_alive || !success) {
    service->CloseService();
  }
}


RefH2Context H2Context::New(const RefCodecMessage& req) {
  return RefH2Context(new H2Context(req));
}

H2Context::H2Context(const RefCodecMessage& request)
  : HttpRequestCtx(request) {
}

void H2Context::Push(const std::string& method,
                     const std::string& path,
                     const HttpRequest* bind_req,
                     const RefHttpResponse& resp,
                     std::function<void(int status)> callback) {

  const HttpRequest* request = Request();
  auto codec = request->GetIOCtx().codec.lock();
  if (!codec) {
    LOG(ERROR) << __FUNCTION__ << " Connection Broken";
    return;
  }
  H2CodecService* h2_con = (H2CodecService*)codec.get();
  if (!h2_con->IOLoop()->IsInLoopThread()) {
    return;
  }
  h2_con->PushPromise(method, path, bind_req, resp, callback);
}

}  // namespace net
}  // namespace lt
