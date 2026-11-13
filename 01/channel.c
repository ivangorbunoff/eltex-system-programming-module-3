#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "channel.h"

enum {
    FD_READ = 0,
    FD_WRITE = 1
};

static int create_pipe_pair(ChannelPair* pair) {
    if (pipe(pair->parent_to_child) == -1) {
        return -1;
    }

    if (pipe(pair->child_to_parent) == -1) {
        int saved_errno = errno;

        close(pair->parent_to_child[FD_READ]);
        close(pair->parent_to_child[FD_WRITE]);

        errno = saved_errno;

        return -1;
    }

    pair->mode = CHANNEL_PIPE;

    return 0;
}

static int create_fifo_pair(ChannelPair* pair, const char* fifo_name) { 
    int path_length = snprintf(
        pair->parent_to_child_path,
        sizeof(pair->parent_to_child_path),
        "%s.p2c",
        fifo_name
    );

    if (path_length < 0 || (size_t) path_length >= sizeof(pair->parent_to_child_path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    path_length = snprintf(
        pair->child_to_parent_path,
        sizeof(pair->child_to_parent_path),
        "%s.c2p",
        fifo_name
    );

    if (path_length < 0 || (size_t) path_length >= sizeof(pair->child_to_parent_path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    if (mkfifo(pair->parent_to_child_path, 0600) == -1) {
        return -1;
    }

    if (mkfifo(pair->child_to_parent_path, 0600) == -1) {
        int saved_errno = errno;

        unlink(pair->parent_to_child_path);
        errno = saved_errno;

        return -1;
    }

    pair->mode = CHANNEL_FIFO;

    return 0;
}

int channel_pair_create(ChannelPair* pair, const char* fifo_name) {
    if (fifo_name != NULL) {
        return create_fifo_pair(pair, fifo_name);
    }

    return create_pipe_pair(pair);
}

int channel_open_parent(ChannelPair* pair, ChannelEndpoint* endpoint) {
    if (pair->mode == CHANNEL_PIPE) {
        endpoint->read_fd = pair->child_to_parent[FD_READ];
        endpoint->write_fd = pair->parent_to_child[FD_WRITE];

        close(pair->parent_to_child[FD_READ]);
        close(pair->child_to_parent[FD_WRITE]);

        return 0;
    }

    endpoint->write_fd = open(pair->parent_to_child_path, O_WRONLY);

    if (endpoint->write_fd == -1) {
        return -1;
    }

    endpoint->read_fd = open(pair->child_to_parent_path, O_RDONLY);

    if (endpoint->read_fd == -1) {
        int saved_errno = errno;

        close(endpoint->write_fd);
        errno = saved_errno;

        return -1;
    }

    return 0;
}

int channel_open_child(ChannelPair* pair, ChannelEndpoint* endpoint) {
    if (pair->mode == CHANNEL_PIPE) {
        endpoint->read_fd = pair->parent_to_child[FD_READ];
        endpoint->write_fd = pair->child_to_parent[FD_WRITE];

        close(pair->parent_to_child[FD_WRITE]);
        close(pair->child_to_parent[FD_READ]);

        return 0;
    }

    endpoint->read_fd = open(pair->parent_to_child_path, O_RDONLY);

    if (endpoint->read_fd == -1) {
        return -1;
    }

    endpoint->write_fd = open(pair->child_to_parent_path, O_WRONLY);

    if (endpoint->write_fd == -1) {
        int saved_errno = errno;

        close(endpoint->read_fd);
        errno = saved_errno;

        return -1;
    }

    return 0;
}

void channel_endpoint_close(ChannelEndpoint* endpoint) {
    close(endpoint->read_fd);
    close(endpoint->write_fd);
}

void channel_pair_cleanup(ChannelPair* pair) {
    if (pair->mode == CHANNEL_FIFO) {
        unlink(pair->parent_to_child_path);
        unlink(pair->child_to_parent_path);
    }
}