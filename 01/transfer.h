#ifndef TRANSFER_H
#define TRANSFER_H

#include "channel.h"

int parent_send_files(ChannelEndpoint* endpoint, char* file_names[], int file_count);

int child_receive_files(ChannelEndpoint* endpoint);

#endif