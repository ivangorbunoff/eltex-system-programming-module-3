#ifndef UI_H
#define UI_H

#include <sys/types.h>

void ui_show_usage(const char* program_name);

void ui_show_error(const char* context);

void ui_show_copied(const char* file_name, off_t file_size);

#endif