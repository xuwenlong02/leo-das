#ifndef SRC_COMPONENTS_UTILS_INCLUDE_UTILS_SIGNALS_H_
#define SRC_COMPONENTS_UTILS_INCLUDE_UTILS_SIGNALS_H_

#ifdef __QNXNTO__
typedef void (*sighandler_t)(int);
#else
#include <signal.h>
#endif
#include <defines.h>

namespace utils {
BOOL UnsibscribeFromTermination();
BOOL WaitTerminationSignals(sighandler_t sig_handler);
}

#endif  // SRC_COMPONENTS_UTILS_INCLUDE_UTILS_SIGNALS_H_
