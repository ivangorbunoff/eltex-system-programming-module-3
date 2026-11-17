#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "queues.h"

int queue_pair_open(QueuePair* pair, const char* base_name) {
    struct mq_attr attributes = {0};

    attributes.mq_maxmsg = QUEUE_CAPACITY;
    attributes.mq_msgsize = MESSAGE_SIZE;

    int first_length = snprintf(pair->first_name, sizeof(pair->first_name), "%s_1", base_name);
    int second_length = snprintf(pair->second_name, sizeof(pair->second_name), "%s_2", base_name);

    if (first_length < 0 || second_length < 0) {
        return -1;
    }

    if (first_length >= QUEUE_NAME_LENGTH || second_length >= QUEUE_NAME_LENGTH) {
        errno = ENAMETOOLONG;
        return -1;
    }

    mqd_t first_queue = mq_open(
        pair->first_name,
        O_CREAT | O_EXCL | O_RDONLY,
        0600,
        &attributes
    );

    if (first_queue != (mqd_t) -1) {
        mqd_t second_queue = mq_open(
            pair->second_name,
            O_CREAT | O_EXCL | O_WRONLY,
            0600,
            &attributes
        );

        if (second_queue == (mqd_t) -1) {
            mq_close(first_queue);
            mq_unlink(pair->first_name);
            return -1;
        }

        pair->incoming = first_queue;
        pair->outgoing = second_queue;
        pair->is_creator = 1;

        return 0;
    }

    if (errno == EEXIST) {
        first_queue = mq_open(pair->first_name, O_WRONLY);

        if (first_queue == (mqd_t) -1) {
            return -1;
        }

        mqd_t second_queue = mq_open(pair->second_name, O_RDONLY);

        if (second_queue == (mqd_t) -1) {
            mq_close(first_queue);
            return -1;
        }

        pair->incoming = second_queue;
        pair->outgoing = first_queue;
        pair->is_creator = 0;

        return 0;
    }

    return -1;
}

int queue_pair_send(const QueuePair* pair, const char* text, unsigned priority) {
    size_t message_length = strlen(text) + 1;

    return mq_send(pair->outgoing, text, message_length, priority);
}

ssize_t queue_pair_receive(const QueuePair* pair, char* buffer, unsigned* priority) {
    return mq_receive(pair->incoming, buffer, MESSAGE_SIZE, priority);
}

void queue_pair_close(const QueuePair* pair) {
    mq_close(pair->incoming);
    mq_close(pair->outgoing);
}

int queue_pair_unlink(const QueuePair* pair) {
    int first_result = mq_unlink(pair->first_name);
    int second_result = mq_unlink(pair->second_name);

    if (first_result == -1 || second_result == -1) {
        return -1;
    }

    return 0;
}