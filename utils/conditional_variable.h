#ifndef SRC_COMPONENTS_INCLUDE_UTILS_CONDITIONAL_VARIABLE_H_
#define SRC_COMPONENTS_INCLUDE_UTILS_CONDITIONAL_VARIABLE_H_

#include <pthread.h>
#include <stdint.h>
#include <macro.h>

class AutoLock;
class Lock;


/*
 * Conditional variable wrapper
 * Conditional variable is a thing that can be waited on
 * Wait is finished when other thread puts that thing in a signaled state
 * (or when timeout is over).
 * Data that is conditionally accessed should be protected by
 * a Lock and that lock must be taken before starting to Wait.
 * When wait is performed, Lock is temporarly released.
 * When wait is finished, Lock is captured back.
 * WARNING: Beware of Spurious wakeups
 * http://en.wikipedia.org/wiki/Spurious_wakeup
 * Thread can wake up from wait spuriously, without conditional
 * variable being actually set by other thread. This means
 * additional check should be made right after thread awakening
 * and if check fails thread should continue waiting.
 *
 * while(!DataReady()) cond_var.Wait(auto_lock);
 *
 */
class ConditionalVariable {
 public:
  enum WaitStatus { kNoTimeout, kTimeout };
  ConditionalVariable();
  ~ConditionalVariable();
  // Wakes up single thread that is waiting on this conditional variable
  void NotifyOne();
  // Wakes up all waiting threads
  void Broadcast();

  // Wait forever or up to milliseconds time limit
  bool Wait(AutoLock& auto_lock);
  bool Wait(Lock& lock);
  WaitStatus WaitFor(AutoLock& auto_lock, uint32_t milliseconds);

 private:
  pthread_cond_t cond_var_;

 private:
  DISALLOW_COPY_AND_ASSIGN(ConditionalVariable);
};  // namespace sync_primitives
#endif  // SRC_COMPONENTS_INCLUDE_UTILS_CONDITIONAL_VARIABLE_H_
