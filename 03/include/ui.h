#ifndef UI_H
#define UI_H

#include "queues.h"

void ui_show_usage(const char* program_name);

void ui_show_error(const char* context);

void ui_show_connection(const QueuePair* pair);

void ui_show_prompt(void);

void ui_show_message(const char* text);

void ui_show_status(const char* text);

#endif
