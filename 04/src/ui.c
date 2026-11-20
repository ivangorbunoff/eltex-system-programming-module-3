#include <stdio.h>

#include "ui.h"

void ui_show_usage(const char* program_name) {
    fprintf(stderr, "Использование:\n");
    fprintf(stderr, "  %s -p\n", program_name);
    fprintf(stderr, "  %s -c\n", program_name);
}

void ui_show_error(const char* context) {
    perror(context);
}

void ui_show_producer_started(int shared_memory_id, int semaphore_id) {
    printf("Производитель запущен\n");
    printf("ID разделяемой памяти: %d\n", shared_memory_id);
    printf("ID семафора: %d\n", semaphore_id);
    fflush(stdout);
}

void ui_show_consumer_started(pid_t consumer_pid) {
    printf("Потребитель запущен, PID: %ld\n", (long) consumer_pid);
    fflush(stdout);
}

void ui_show_generated_block(const int* numbers, size_t number_count) {
    printf("Создан блок из %zu чисел: ", number_count);

    for (size_t index = 0; index < number_count; index++) {
        printf("%d ", numbers[index]);
    }

    printf("\n");
    fflush(stdout);
}

void ui_show_processed_block(pid_t consumer_pid, size_t number_count, int minimum, int maximum) {
    printf(
        "Потребитель PID %ld: обработано %zu чисел:\nmin = %d,\nmax = %d\n",
        (long) consumer_pid,
        number_count,
        minimum,
        maximum
    );

    fflush(stdout);
}

void ui_show_status(const char* text) {
    printf("%s\n", text);
    fflush(stdout);
}
