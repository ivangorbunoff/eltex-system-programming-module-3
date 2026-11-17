#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "ipc.h"
#include "protocol.h"
#include "signal_control.h"
#include "subscriber.h"
#include "ui.h"

static int send_topic_command(int queue_id, const char* command, pid_t pid, const char* topic) {
    char message[MAX_MESSAGE_LENGTH];

    snprintf(message, sizeof(message), "%s,%ld,%s", command, (long) pid, topic);

    return ipc_send(queue_id, BROKER_MESSAGE_TYPE, message);
}

int run_subscriber(char* const topics[], int topic_count) {
    QueueMessage message;
    ParsedCommand command;
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

    for (int index = 0; index < topic_count; index++) {
        if (send_topic_command(queue_id, "subscribe", pid, topics[index]) == -1) {
            ui_show_error("Не удалось оформить подписку");
            return 1;
        }
    }

    ui_show_started("Подписчик", pid);

    while (!stop_requested) {
        ssize_t received = ipc_receive(queue_id, (long) pid, &message, 0);

        if (received == -1) {
            if (errno == EINTR) {
                continue;
            }

            ui_show_error("Не удалось получить сообщение");
            exit_code = 1;
            break;
        }

        if (protocol_parse(message.text, &command) == -1) {
            continue;
        }

        if (command.type == COMMAND_SEND) {
            ui_show_message(command.pid, command.topic, command.payload);
        }
    }

    for (int index = 0; index < topic_count; index++) {
        if (send_topic_command(queue_id, "unsubscribe", pid, topics[index]) == -1) {
            break;
        }
    }

    ui_show_stopped("Подписчик");

    return exit_code;
}