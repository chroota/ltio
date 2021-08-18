#include "co_mutex.h"
#include "co_runner.h"

using base::LtClosure;

namespace co {

CoMutex::CoMutex() : locked_(false) {}
CoMutex::~CoMutex() {}

void CoMutex::lock() {
  CHECK(__co_yielable__);

  lock_.lock();
  if (!locked_) {
    locked_ = true;
    lock_.unlock();
    return;
  }
  waiters_.emplace_back(std::move(co_new_resumer()));
  lock_.unlock();

  __co_wait_here__;
}

void CoMutex::unlock() {
  lock_.lock();
  if (waiters_.empty()) {
    locked_ = false;
    lock_.unlock();
    return;
  }

  LtClosure resumer(std::move(waiters_.front()));
  waiters_.pop_front();
  lock_.unlock();

  resumer();
}

bool CoMutex::try_lock() {
  std::lock_guard<base::SpinLock> guard(lock_);
  return locked_ ? false : (locked_ = true);
}

}  // namespace base
