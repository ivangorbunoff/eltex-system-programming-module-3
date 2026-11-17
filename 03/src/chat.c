#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "chat.h"
#include "queues.h"
#include "signal_control.h"
#include "ui.h"

typedef struct {
    const QueuePair* pair;
    pthread_t main_thread;
} SenderContext;

static void* run_sender(void* argument) {
    SenderContext* context = argument;
    char message[MESSAGE_SIZE];

    while (1) {
        ui_show_prompt();

        if (fgets(message, sizeof(message), stdin) == NULL) {
            pthread_kill(context->main_thread, SIGINT);
            return NULL;
        }

        message[strcspn(message, "\n")] = '\0';

        if (queue_pair_send(context->pair, message, 0) == -1) {
            ui_show_error("Не удалось отправить сообщение");
            pthread_kill(context->main_thread, SIGINT);
            return NULL;
        }
    }
}

int run_chat(const char* base_name) {
    QueuePair pair;
    pthread_t sender_thread;
    char message[MESSAGE_SIZE];
    unsigned int priority;
    int exit_code = 0;
    int peer_finished = 0;

    if (signal_control_install() == -1) {
        ui_show_error("Не удалось установить обработчик SIGINT");
        return 1;
    }

    if (signal_control_block_sigint() == -1) {
        ui_show_error("Не удалось заблокировать SIGINT");
        return 1;
    }

    if (queue_pair_open(&pair, base_name) == -1) {
        ui_show_error("Не удалось открыть очереди");
        return 1;
    }

    ui_show_connection(&pair);

    SenderContext context = {
        .pair = &pair,
        .main_thread = pthread_self()
    };

    int thread_error = pthread_create(&sender_thread, NULL, run_sender, &context);

    if (thread_error != 0) {
        errno = thread_error;
        ui_show_error("Не удалось создать поток отправки");
        queue_pair_close(&pair);

        if (pair.is_creator) {
            queue_pair_unlink(&pair);
        }

        return 1;
    }

    if (signal_control_unblock_sigint() == -1) {
        ui_show_error("Не удалось разблокировать SIGINT");
        pthread_cancel(sender_thread);
        pthread_join(sender_thread, NULL);
        queue_pair_close(&pair);

        if (pair.is_creator) {
            queue_pair_unlink(&pair);
        }

        return 1;
    }

    while (!stop_requested) {
        ssize_t received = queue_pair_receive(&pair, message, &priority);

        if (received == -1) {
            if (errno == EINTR) {
                continue;
            }

            ui_show_error("Не удалось получить сообщение");
            exit_code = 1;
            break;
        }

        if (priority == FINISH_PRIORITY) {
            peer_finished = 1;
            ui_show_status("Собеседник завершает чат");
            break;
        }

        ui_show_message(message);
    }

    if (!peer_finished) {
        if (queue_pair_send(&pair, "", FINISH_PRIORITY) == -1) {
            ui_show_error("Не удалось отправить сообщение о завершении");
            exit_code = 1;
        }
    }

    pthread_cancel(sender_thread);

    int join_error = pthread_join(sender_thread, NULL);

    if (join_error != 0) {
        errno = join_error;
        ui_show_error("Не удалось дождаться потока отправки");
        exit_code = 1;
    }

    queue_pair_close(&pair);

    if (pair.is_creator) {
        if (queue_pair_unlink(&pair) == -1) {
            ui_show_error("Не удалось удалить очереди");
            exit_code = 1;
        }
    }

    ui_show_status("Чат завершён");

    return exit_code;
}
