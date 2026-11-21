#ifndef UI_H
#define UI_H

#include <stddef.h>
#include <sys/types.h>

#include "ipc.h"

void ui_show_usage(const char* program_name);

void ui_show_error(const char* context);

void ui_show_producer_started(void);

void ui_show_consumer_started(pid_t consumer_pid);

void ui_show_generated_block(const int* numbers, size_t number_count);

void ui_show_processed_block(
    pid_t consumer_pid,
    size_t number_count,
    int minimum,
    int maximum
);

void ui_show_status(const char* text);

#endif
