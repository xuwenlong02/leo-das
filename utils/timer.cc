/*
 * Copyright (c) 2016, Ford Motor Company
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following
 * disclaimer in the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of the Ford Motor Company nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "timer.h"

#include <string>

#include "macro.h"
#include "logger.h"
#include "lock.h"
#include "timer_task.h"
#include "conditional_variable.h"
#include "threads/thread.h"
#include "threads/thread_delegate.h"

Timer::Timer(const std::string& name, TimerTask* task)
    : name_(name)
    , task_(task)
    , state_lock_(true)
    , delegate_(new TimerDelegate(this, state_lock_))
    , thread_(threads::CreateThread(name_.c_str(), delegate_.get()))
    , single_shot_(true) {
  DCHECK(!name_.empty());
  DCHECK(task_);
  DCHECK(thread_);
}

Timer::~Timer() {
  AutoLock auto_lock(state_lock_);
  StopThread();
  StopDelegate();
  single_shot_ = true;

  delegate_.reset();
  threads::DeleteThread(thread_);
  LOG_DEBUG("Timer %s has been destroyed", name_.c_str());
}

void Timer::Start(const Milliseconds timeout,
                         const TimerType timer_type) {
  AutoLock auto_lock(state_lock_);
  StopThread();
  switch (timer_type) {
    case kSingleShot: {
      single_shot_ = true;
      break;
    }
    case kPeriodic: {
      single_shot_ = false;
      break;
    }
    default: { ASSERT("timer_type should be kSingleShot or kPeriodic"); }
  };
  StartDelegate(timeout);
  StartThread();
  LOG_DEBUG("Timer %s has been startd",name_.c_str());
}

void Timer::Stop() {
  AutoLock auto_lock(state_lock_);
  StopThread();
  StopDelegate();
  single_shot_ = true;
  LOG_DEBUG("Timer %s has been stopped",name_.c_str());
}

bool Timer::is_running() const {
  AutoLock auto_lock(state_lock_);
  return !delegate_->stop_flag();
}

bool Timer::IsRunning() const
{
    return is_running();
}

Milliseconds Timer::timeout() const {
  AutoLock auto_lock(state_lock_);
  return delegate_->timeout();
}

void Timer::set_timeout(Milliseconds timeout)
{
    delegate_->set_timeout(timeout);
}

void Timer::StartDelegate(const Milliseconds timeout) const {
  delegate_->set_stop_flag(false);
  delegate_->set_timeout(timeout);
}

void Timer::StopDelegate() const {
  delegate_->set_stop_flag(true);
  delegate_->set_timeout(0);
}

void Timer::StartThread() {
  if (delegate_->finalized_flag()) {
    return;
  }

  DCHECK_OR_RETURN_VOID(thread_);
  if (!thread_->IsCurrentThread()) {
    thread_->start();
  }
}

void Timer::StopThread() {
  if (delegate_->finalized_flag()) {
    return;
  }

  DCHECK_OR_RETURN_VOID(thread_);
  if (!thread_->IsCurrentThread()) {
    delegate_->set_finalized_flag(true);
    {
      AutoUnlock auto_unlock(state_lock_);
      thread_->join();
    }
    delegate_->set_finalized_flag(false);
  }
}

void Timer::OnTimeout() const {
  {
    AutoLock auto_lock(state_lock_);
    if (single_shot_) {
      StopDelegate();
    }
  }

  DCHECK_OR_RETURN_VOID(task_);
  task_->RunTimer();
}

Timer::TimerDelegate::TimerDelegate(
    const Timer* timer,Lock& state_lock_ref)
    : timer_(timer)
    , timeout_(0)
    , stop_flag_(true)
    , finalized_flag_(false)
    , state_lock_ref_(state_lock_ref)
    , state_condition_() {
  DCHECK(timer_);
}

void Timer::TimerDelegate::set_timeout(const Milliseconds timeout) {
  timeout_ = timeout;
}

Milliseconds Timer::TimerDelegate::timeout() const {
  return timeout_;
}

void Timer::TimerDelegate::set_stop_flag(const bool stop_flag) {
  stop_flag_ = stop_flag;
}

bool Timer::TimerDelegate::stop_flag() const {
  return stop_flag_;
}

void Timer::TimerDelegate::set_finalized_flag(
    const bool finalized_flag) {
  finalized_flag_ = finalized_flag;
}

bool Timer::TimerDelegate::finalized_flag() const {
  return finalized_flag_;
}

void Timer::TimerDelegate::threadMain() {
  AutoLock auto_lock(state_lock_ref_);
  while (!stop_flag_ && !finalized_flag_) {
    if (ConditionalVariable::kTimeout ==
        state_condition_.WaitFor(auto_lock, timeout_)) {
      if (timer_) {
        AutoUnlock auto_unlock(auto_lock);
        timer_->OnTimeout();
      }
    } else {
      LOG_DEBUG("Timer has been force reset");
    }
  }
}

void Timer::TimerDelegate::exitThreadMain() {
  AutoLock auto_lock(state_lock_ref_);
  state_condition_.NotifyOne();
}
