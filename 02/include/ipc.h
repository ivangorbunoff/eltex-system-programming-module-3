#ifndef IPC_H
#define IPC_H

#include <sys/types.h>

#define BROKER_MESSAGE_TYPE 1L
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
    long type;
    char text[MAX_MESSAGE_LENGTH];
} QueueMessage;

int ipc_create_queue(void);

int ipc_open_queue(void);

int ipc_send(int queue_id, long type, const char* text);

ssize_t ipc_receive(int queue_id, long type, QueueMessage* message, int flags);

int ipc_remove_queue(int queue_id);

#endif