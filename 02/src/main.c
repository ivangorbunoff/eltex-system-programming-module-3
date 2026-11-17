#include <string.h>

#include "broker.h"
#include "protocol.h"
#include "publisher.h"
#include "subscriber.h"
#include "ui.h"

static int topic_is_valid(const char* topic) {
    if (topic[0] == '\0') {
        return 0;
    }

    if (strlen(topic) >= MAX_TOPIC_LENGTH) {
        return 0;
    }

    if (strchr(topic, ',') != NULL) {
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && strcmp(argv[1], "-b") == 0) {
        return run_broker();
    }

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {
        if (!topic_is_valid(argv[2])) {
            ui_show_usage(argv[0]);
            return 1;
        }

        return run_publisher(argv[2]);
    }

    if (argc >= 3 && strcmp(argv[1], "-s") == 0) {
        for (int index = 2; index < argc; index++) {
            if (!topic_is_valid(argv[index])) {
                ui_show_usage(argv[0]);
                return 1;
            }
        }

        return run_subscriber(argv + 2, argc - 2);
    }

    ui_show_usage(argv[0]);

    return 1;
}