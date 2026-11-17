#include <stdio.h>
#include <string.h>

#include "protocol.h"

int protocol_parse(const char* text, ParsedCommand* command) {
    long pid;
    char extra;

    memset(command, 0, sizeof(*command));

    if (sscanf(text, "register,%ld%c", &pid, &extra) == 1 && pid > 0) {
        command->type = COMMAND_REGISTER;
        command->pid = (pid_t) pid;
        return 0;
    }

    if (sscanf(text, "subscribe,%ld,%63[^,\n]%c", &pid, command->topic, &extra) == 2 && pid > 0) {
        command->type = COMMAND_SUBSCRIBE;
        command->pid = (pid_t) pid;
        return 0;
    }

    if (sscanf(text, "unsubscribe,%ld,%63[^,\n]%c", &pid, command->topic, &extra) == 2 && pid > 0) {
        command->type = COMMAND_UNSUBSCRIBE;
        command->pid = (pid_t) pid;
        return 0;
    }

    char separator;
    int payload_offset;

    int matched = sscanf(
        text,
        "send,%ld,%63[^,\n]%c%n",
        &pid,
        command->topic,
        &separator,
        &payload_offset
    );

    if (matched == 3 && separator == '\n' && pid > 0) {
        const char* payload = text + payload_offset;
        size_t payload_length = strlen(payload);

        if (payload_length >= sizeof(command->payload)) {
            return -1;
        }

        command->type = COMMAND_SEND;
        command->pid = (pid_t) pid;
        memcpy(command->payload, payload, payload_length + 1);

        return 0;
    }

    return -1;
}