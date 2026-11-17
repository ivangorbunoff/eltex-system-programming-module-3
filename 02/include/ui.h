#ifndef UI_H
#define UI_H

#include <sys/types.h>

void ui_show_usage(const char* program_name);

void ui_show_error(const char* context);

void ui_show_started(const char* role, pid_t pid);

void ui_show_stopped(const char* role);

void ui_show_prompt(const char* topic);

void ui_show_message(pid_t publisher_pid, const char* topic, const char* payload);

#endif