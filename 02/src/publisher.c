#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ipc.h"
#include "publisher.h"
#include "signal_control.h"
#include "ui.h"

#define INPUT_LENGTH 512

int run_publisher(const char* topic) {
    char registration[64];
    char payload[INPUT_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
    int exit_code = 0;
    pid_t pid = getpid();

    int queue_id = ipc_open_queue();

    if (queue_id == -1) {
        ui_show_error("Не удалось открыть очередь");
        return 1;
    }

    if (signal_control_install() == -1) {
        ui_show_error("Не удалось установить обработчики сигналов");
        return 1;
    }

    snprintf(registration, sizeof(registration), "register,%ld", (long) pid);

    if (ipc_send(queue_id, BROKER_MESSAGE_TYPE, registration) == -1) {
        ui_show_error("Не удалось зарегистрировать издателя");
        return 1;
    }

    ui_show_started("Издатель", pid);

    while (!stop_requested) {
        ui_show_prompt(topic);

        if (fgets(payload, sizeof(payload), stdin) == NULL) {
            if (stop_requested || feof(stdin)) {
                break;
            }

            ui_show_error("Не удалось прочитать сообщение");
            exit_code = 1;
            break;
        }

        payload[strcspn(payload, "\n")] = '\0';

        if (strcmp(payload, "/quit") == 0) {
            break;
        }

        snprintf(
            message,
            sizeof(message),
            "send,%ld,%s\n%s",
            (long) pid,
            topic,
            payload
        );

        if (ipc_send(queue_id, BROKER_MESSAGE_TYPE, message) == -1) {
            if (errno == EINTR && stop_requested) {
                break;
            }

            ui_show_error("Не удалось отправить сообщение");
            exit_code = 1;
            break;
        }
    }

    ui_show_stopped("Издатель");

    return exit_code;
}