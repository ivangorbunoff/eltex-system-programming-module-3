#include <stddef.h>

#include "signal_control.h"

volatile sig_atomic_t stop_requested = 0;
volatile sig_atomic_t timeout_expired = 0;

static void handle_stop(int signal_number) {
    (void) signal_number;
    stop_requested = 1;
}

static void handle_timeout(int signal_number) {
    (void) signal_number;
    timeout_expired = 1;
}

int signal_control_install(void) {
    struct sigaction action = {0};

    sigemptyset(&action.sa_mask);
    action.sa_handler = handle_stop;

    if (sigaction(SIGINT, &action, NULL) == -1) {
        return -1;
    }

    action.sa_handler = handle_timeout;

    return sigaction(SIGALRM, &action, NULL);
}