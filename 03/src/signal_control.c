#include <errno.h>
#include <stddef.h>

#include "signal_control.h"

volatile sig_atomic_t stop_requested = 0;

static void handle_sigint(int signal_number) {
    (void) signal_number;
    stop_requested = 1;
}

int signal_control_install(void) {
    struct sigaction action = {0};

    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);

    return sigaction(SIGINT, &action, NULL);
}

static int change_sigint_mask(int operation) {
    sigset_t signal_set;

    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGINT);

    int error = pthread_sigmask(operation, &signal_set, NULL);

    if (error != 0) {
        errno = error;
        return -1;
    }

    return 0;
}

int signal_control_block_sigint(void) {
    return change_sigint_mask(SIG_BLOCK);
}

int signal_control_unblock_sigint(void) {
    return change_sigint_mask(SIG_UNBLOCK);
}
