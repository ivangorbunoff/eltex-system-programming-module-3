#ifndef SIGNAL_CONTROL_H
#define SIGNAL_CONTROL_H

#include <signal.h>

extern volatile sig_atomic_t stop_requested;

extern volatile sig_atomic_t timeout_expired;

int signal_control_install(void);

#endif