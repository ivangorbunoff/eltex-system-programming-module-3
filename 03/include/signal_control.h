#ifndef SIGNAL_CONTROL_H
#define SIGNAL_CONTROL_H

#include <signal.h>

extern volatile sig_atomic_t stop_requested;

int signal_control_install(void);

int signal_control_block_sigint(void);

int signal_control_unblock_sigint(void);

#endif
