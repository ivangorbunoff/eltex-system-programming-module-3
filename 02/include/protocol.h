#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <sys/types.h>

#include "ipc.h"

#define MAX_TOPIC_LENGTH 64

typedef enum {
    COMMAND_REGISTER,
    COMMAND_SUBSCRIBE,
    COMMAND_UNSUBSCRIBE,
    COMMAND_SEND
} CommandType;

typedef struct {
    CommandType type;
    pid_t pid;
    char topic[MAX_TOPIC_LENGTH];
    char payload[MAX_MESSAGE_LENGTH];
} ParsedCommand;

int protocol_parse(const char* text, ParsedCommand* command);

#endif