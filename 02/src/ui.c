#include <stdio.h>

#include "ui.h"

void ui_show_usage(const char* program_name) {
    fprintf(stderr, "Использование:\n");
    fprintf(stderr, "  %s -b\n", program_name);
    fprintf(stderr, "  %s -p <тема>\n", program_name);
    fprintf(stderr, "  %s -s <тема> [тема ...]\n", program_name);
}

void ui_show_error(const char* context) {
    perror(context);
}

void ui_show_started(const char* role, pid_t pid) {
    printf("%s запущен, PID: %ld\n", role, (long) pid);
    fflush(stdout);
}

void ui_show_stopped(const char* role) {
    printf("%s завершён\n", role);
    fflush(stdout);
}

void ui_show_prompt(const char* topic) {
    printf("[%s] > ", topic);
    fflush(stdout);
}

void ui_show_message(pid_t publisher_pid, const char* topic, const char* payload) {
    printf("[%s] от PID %ld: %s\n", topic, (long) publisher_pid, payload);
    fflush(stdout);
}