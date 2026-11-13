#include <stdio.h>

#include "ui.h"

void ui_show_usage(const char* program_name) {
    fprintf(stderr, "Использование: %s [-p имя_канала] файл...\n", program_name);
}

void ui_show_error(const char* context) {
    perror(context);
}

void ui_show_copied(const char* file_name, off_t file_size) {
    printf("Скопирован: %s -> %s.copy (%lld байт)\n", file_name, file_name, (long long) file_size);
    fflush(stdout);
}