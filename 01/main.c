#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "channel.h"
#include "transfer.h"
#include "ui.h"

static void run_child(ChannelPair* pair) {
    ChannelEndpoint endpoint;

    if (channel_open_child(pair, &endpoint) == -1) {
        ui_show_error("Не удалось открыть каналы ребёнка");
        _exit(EXIT_FAILURE);
    }

    int result = child_receive_files(&endpoint);

    channel_endpoint_close(&endpoint);

    _exit(result == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

static int run_parent(ChannelPair* pair, pid_t child_pid, char* file_names[], int file_count) {
    ChannelEndpoint endpoint;

    if (channel_open_parent(pair, &endpoint) == -1) {
        ui_show_error("Не удалось открыть каналы родителя");
        channel_pair_cleanup(pair);
        return EXIT_FAILURE;
    }

    int transfer_result = parent_send_files(&endpoint, file_names, file_count);

    channel_endpoint_close(&endpoint);

    int child_status;

    if (waitpid(child_pid, &child_status, 0) == -1) {
        ui_show_error("waitpid");
        channel_pair_cleanup(pair);
        return EXIT_FAILURE;
    }

    channel_pair_cleanup(pair);

    if (transfer_result == -1) {
        return EXIT_FAILURE;
    }

    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char* argv[]) {
    const char* fifo_name = NULL;
    int first_file_index = 1;

    if (argc > 1 && strcmp(argv[1], "-p") == 0) {
        if (argc < 4) {
            ui_show_usage(argv[0]);
            return EXIT_FAILURE;
        }

        fifo_name = argv[2];
        first_file_index = 3;
    }

    if (first_file_index >= argc) {
        ui_show_usage(argv[0]);
        return EXIT_FAILURE;
    }

    ChannelPair pair;

    if (channel_pair_create(&pair, fifo_name) == -1) {
        ui_show_error("Не удалось создать каналы");
        return EXIT_FAILURE;
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        ui_show_error("fork");
        channel_pair_cleanup(&pair);
        return EXIT_FAILURE;
    }

    if (child_pid == 0) {
        run_child(&pair);
    }

    return run_parent(
        &pair,
        child_pid,
        argv + first_file_index,
        argc - first_file_index
    );
}