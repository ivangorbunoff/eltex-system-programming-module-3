#include <errno.h>
#include <signal.h>
#include <sys/msg.h>
#include <unistd.h>

#include "broker.h"
#include "ipc.h"
#include "protocol.h"
#include "registry.h"
#include "signal_control.h"
#include "ui.h"

static int forward_message(
    int queue_id,
    const Registry* registry,
    const ParsedCommand* command,
    const char* message_text
) {
    const SubscriberNode* subscriber = registry->subscribers;

    while (subscriber != NULL) {
        if (subscriber_has_topic(subscriber, command->topic)) {
            if (ipc_send(queue_id, (long) subscriber->pid, message_text) == -1) {
                return -1;
            }
        }

        subscriber = subscriber->next;
    }

    return 0;
}

static void notify_clients(const Registry* registry) {
    const SubscriberNode* subscriber = registry->subscribers;
    const PublisherNode* publisher = registry->publishers;

    while (subscriber != NULL) {
        kill(subscriber->pid, SIGINT);
        subscriber = subscriber->next;
    }

    while (publisher != NULL) {
        kill(publisher->pid, SIGINT);
        publisher = publisher->next;
    }
}

static void drain_queue(int queue_id) {
    QueueMessage message;

    alarm(3);

    while (!timeout_expired) {
        if (ipc_receive(queue_id, 0, &message, IPC_NOWAIT) >= 0) {
            continue;
        }

        if (errno == ENOMSG) {
            break;
        }

        if (errno == EINTR) {
            continue;
        }

        break;
    }

    alarm(0);
}

int run_broker(void) {
    Registry registry = {0};
    QueueMessage message;
    ParsedCommand command;
    int exit_code = 0;

    int queue_id = ipc_create_queue();

    if (queue_id == -1) {
        if (errno == EEXIST) {
            ui_show_error("Брокер уже запущен");
        } else {
            ui_show_error("Не удалось создать очередь");
        }

        return 1;
    }

    if (signal_control_install() == -1) {
        ui_show_error("Не удалось установить обработчики сигналов");
        ipc_remove_queue(queue_id);
        return 1;
    }

    ui_show_started("Брокер", getpid());

    while (!stop_requested && exit_code == 0) {
        if (ipc_receive(queue_id, BROKER_MESSAGE_TYPE, &message, 0) == -1) {
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

        switch (command.type) {
            case COMMAND_REGISTER:
                if (registry_register_publisher(&registry, command.pid) == -1) {
                    ui_show_error("Не удалось зарегистрировать издателя");
                    exit_code = 1;
                }
                break;

            case COMMAND_SUBSCRIBE:
                if (registry_add_subscription(&registry, command.pid, command.topic) == -1) {
                    ui_show_error("Не удалось добавить подписку");
                    exit_code = 1;
                }
                break;

            case COMMAND_UNSUBSCRIBE:
                registry_remove_subscription(&registry, command.pid, command.topic);
                break;

            case COMMAND_SEND:
                if (forward_message(queue_id, &registry, &command, message.text) == -1) {
                    ui_show_error("Не удалось переслать сообщение");
                    exit_code = 1;
                }
                break;
        }
    }

    notify_clients(&registry);
    drain_queue(queue_id);

    if (ipc_remove_queue(queue_id) == -1) {
        ui_show_error("Не удалось удалить очередь");
        exit_code = 1;
    }

    registry_clear(&registry);
    ui_show_stopped("Брокер");

    return exit_code;
}