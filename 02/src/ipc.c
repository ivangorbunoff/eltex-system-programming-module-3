#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "ipc.h"

static key_t get_queue_key(void) {
    return ftok(".", 'E');
}

int ipc_create_queue(void) {
    key_t key = get_queue_key();

    if (key == -1) {
        return -1;
    }

    return msgget(key, IPC_CREAT | IPC_EXCL | 0600);
}

int ipc_open_queue(void) {
    key_t key = get_queue_key();

    if (key == -1) {
        return -1;
    }

    return msgget(key, 0);
}

int ipc_send(int queue_id, long type, const char* text) {
    QueueMessage message;
    size_t text_length = strlen(text);

    if (text_length >= sizeof(message.text)) {
        errno = EMSGSIZE;
        return -1;
    }

    message.type = type;
    memcpy(message.text, text, text_length + 1);

    return msgsnd(queue_id, &message, text_length + 1, 0);
}

ssize_t ipc_receive(int queue_id, long type, QueueMessage* message, int flags) {
    return msgrcv(queue_id, message, sizeof(message->text), type, flags);
}

int ipc_remove_queue(int queue_id) {
    return msgctl(queue_id, IPC_RMID, NULL);
}