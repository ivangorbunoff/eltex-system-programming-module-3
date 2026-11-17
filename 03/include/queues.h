#ifndef QUEUES_H
#define QUEUES_H

#include <mqueue.h>
#include <sys/types.h>

#define QUEUE_NAME_LENGTH 256
#define MESSAGE_SIZE 512
#define QUEUE_CAPACITY 10

#define FINISH_PRIORITY 10U

typedef struct {
    mqd_t incoming;
    mqd_t outgoing;
    char first_name[QUEUE_NAME_LENGTH];
    char second_name[QUEUE_NAME_LENGTH];
    int is_creator;
} QueuePair;

int queue_pair_open(QueuePair* pair, const char* base_name);

int queue_pair_send(const QueuePair* pair, const char* text, unsigned priority);

ssize_t queue_pair_receive(const QueuePair* pair, char* buffer, unsigned* priority);

void queue_pair_close(const QueuePair* pair);

int queue_pair_unlink(const QueuePair* pair);

#endif
