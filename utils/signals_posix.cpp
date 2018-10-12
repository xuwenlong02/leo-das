#include <csignal>
#include <cstdlib>
#include <stdint.h>
#include <event.h>

#include "signals.h"
#include <logger.h>

namespace utils {

BOOL UnsibscribeFromTermination() {
  // Disable some system signals receiving in thread
  // by blocking those signals
  // (system signals processes only in the main thread)
  // Mustn't block all signals!
  // See "Advanced Programming in the UNIX Environment, 3rd Edition"
  // (http://poincare.matf.bg.ac.rs/~ivana//courses/ps/sistemi_knjige/pomocno/apue.pdf,
  // "12.8. Threads and Signals".
  sigset_t signal_set;
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);

  return !pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
}

namespace {
bool CatchSIGSEGV(sighandler_t handler) {
  struct sigaction act;

  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  return !sigaction(SIGSEGV, &act, NULL);
}
}  // namespace

void signal_cb(evutil_socket_t fd,short event,void *arg)
{
    LOG_TRACE();
    struct event_base *base = (struct event_base *)arg;
    event_base_loopbreak(base);
}

BOOL WaitTerminationSignals(sighandler_t sig_handler) {
#if 0
    sigset_t signal_set;
    int sig = -1;

    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);
    sigaddset(&signal_set, SIGTERM);
  //  sigaddset(&signal_set, SIGPIPE);
  //  sigaddset(&signal_set, SIGBUS);
    sigaddset(&signal_set, SIGSEGV);
    sigaddset(&signal_set, SIGFPE);
    sigaddset(&signal_set, SIGABRT);
    sigaddset(&signal_set, SIGKILL);

    if (!CatchSIGSEGV(sig_handler)) {
      return FALSE;
    }

    if (!sigwait(&signal_set, &sig)) {
      sig_handler(sig);
      return TRUE;
    }
    return FALSE;
#else
    LOG_TRACE();
    struct event_base *base;
    base = event_base_new();

    struct event *signal_int = evsignal_new(base,SIGINT,signal_cb,base);/*event_self_cbarg()*/
    if(!signal_int || event_add(signal_int,NULL) < 0)
    {
        LOG_ERR("create or add signal_int failed");
        return false;
    }
    event_base_dispatch(base);
    event_del(signal_int);
    sig_handler(event_get_signal(signal_int));

    event_free(signal_int);
    event_base_free(base);

    return true;

#endif
}

}
