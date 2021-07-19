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

#include "fd_event.h"

#include "glog/logging.h"

#include "base/base_constants.h"

namespace base {

RefFdEvent FdEvent::Create(FdEvent::Handler* handler, int fd, LtEvent events) {
  return std::make_shared<FdEvent>(handler, fd, events);
}

FdEvent::FdEvent(FdEvent::Handler* handler, int fd, LtEvent event)
  : fd_(fd), events_(event), revents_(0), owner_fd_(true), handler_(handler) {}

FdEvent::~FdEvent() {
  if (owner_fd_) {
    ::close(fd_);
  }
}

void FdEvent::SetFdWatcher(Watcher* d) {
  watcher_ = d;
}

LtEvent FdEvent::MonitorEvents() const {
  return events_;
}

void FdEvent::EnableReading() {
  if (IsReadEnable()) {
    return;
  }
  events_ |= LtEv::LT_EVENT_READ;

  notify_watcher();
}

void FdEvent::EnableWriting() {
  if (IsWriteEnable()) {
    return;
  }
  events_ |= LtEv::LT_EVENT_WRITE;

  notify_watcher();
}

void FdEvent::DisableReading() {
  if (!IsReadEnable())
    return;
  events_ &= ~LtEv::LT_EVENT_READ;
  notify_watcher();
}

void FdEvent::DisableWriting() {
  if (!IsWriteEnable())
    return;

  events_ &= ~LtEv::LT_EVENT_WRITE;
  notify_watcher();
}

void FdEvent::notify_watcher() {
  if (watcher_) {
    watcher_->UpdateFdEvent(this);
  }
}

void FdEvent::HandleEvent(LtEvent mask) {
  static const bool kContinue = true;
  revents_ = mask;
  do {
    if (mask & LtEv::LT_EVENT_ERROR) {
      if (kContinue != handler_->HandleError(this)) {
        break;
      }
    }
    if (mask & LtEv::LT_EVENT_WRITE) {
      if (kContinue != handler_->HandleWrite(this)) {
        break;
      }
    }
    if (mask & LtEv::LT_EVENT_READ) {
      if (kContinue != handler_->HandleRead(this)) {
        break;
      }
    }
    if (mask & LtEv::LT_EVENT_CLOSE) {
      if (kContinue != handler_->HandleClose(this)) {
        break;
      }
    }
  } while (0);
}

std::string FdEvent::EventInfo() const {
  std::ostringstream oss;
  oss << " [fd:" << fd_ << ", watch:" << events2string(events_) << "]";
  return oss.str();
}

std::string FdEvent::RcvEventAsString() const {
  return events2string(revents_);
}

std::string FdEvent::MonitorEventAsString() const {
  return events2string(events_);
}

}  // namespace base
