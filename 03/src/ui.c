#include <stdio.h>

#include "ui.h"
#include "queues.h"

void ui_show_usage(const char* program_name) {
    fprintf(stderr, "Использование: %s /имя_очереди\n", program_name);
}

void ui_show_error(const char* context) {
    perror(context);
}

void ui_show_connection(const QueuePair* pair) {
    const char* incoming_name;
    const char* outgoing_name;

    if (pair->is_creator) {
        incoming_name = pair->first_name;
        outgoing_name = pair->second_name;
        printf("Очереди созданы\n");
    } else {
        incoming_name = pair->second_name;
        outgoing_name = pair->first_name;
        printf("Подключение к существующим очередям\n");
    }

    printf("Приём:    %s\n", incoming_name);
    printf("Отправка: %s\n", outgoing_name);
    fflush(stdout);
}

void ui_show_prompt(void) {
    printf("> ");
    fflush(stdout);
}

void ui_show_message(const char* text) {
    printf("\nСобеседник: %s\n", text);
    fflush(stdout);
}

void ui_show_status(const char* text) {
    printf("%s\n", text);
    fflush(stdout);
}
