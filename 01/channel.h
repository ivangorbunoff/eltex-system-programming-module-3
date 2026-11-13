#ifndef CHANNEL_H
#define CHANNEL_H

#include <linux/limits.h>

typedef enum {
    CHANNEL_PIPE,
    CHANNEL_FIFO
} ChannelMode;

typedef struct {
    int read_fd;
    int write_fd;
} ChannelEndpoint;

typedef struct {
    ChannelMode mode;
    int parent_to_child[2];
    int child_to_parent[2];
    char parent_to_child_path[PATH_MAX];
    char child_to_parent_path[PATH_MAX];
} ChannelPair;

int channel_pair_create(ChannelPair* pair, const char* fifo_name);

int channel_open_parent(ChannelPair* pair, ChannelEndpoint* endpoint);

int channel_open_child(ChannelPair* pair, ChannelEndpoint* endpoint);

void channel_endpoint_close(ChannelEndpoint* endpoint);

void channel_pair_cleanup(ChannelPair* pair);

#endif