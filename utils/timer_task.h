
#ifndef SRC_COMPONENTS_UTILS_INCLUDE_UTILS_TIMER_TASK_H_
#define SRC_COMPONENTS_UTILS_INCLUDE_UTILS_TIMER_TASK_H_


/**
 * @brief The TimerTask interface
 */
class TimerTask {
 public:
  /**
   * @brief this method calls callback from callee
   */
  virtual void RunTimer() = 0;
};

#endif  // SRC_COMPONENTS_UTILS_INCLUDE_UTILS_TIMER_TASK_H_
