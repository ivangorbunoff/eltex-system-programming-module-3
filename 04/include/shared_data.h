#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stddef.h>

#define SHARED_MEMORY_SIZE 2048

typedef struct {
    size_t number_count;
    size_t next_block_start;
    int numbers[];
} NumberBlock;

typedef struct {
    size_t first_block_start;
    size_t last_block_start;
    size_t free_memory_start;
    int producer_finished;
} SharedDataHeader;

void shared_data_init(unsigned char* memory_base);

int shared_data_add_block(
    unsigned char* memory_base,
    const int* numbers,
    size_t number_count
);

NumberBlock* shared_data_find_unprocessed_block(unsigned char* memory_base);

int shared_data_all_blocks_processed(const unsigned char* memory_base);

#endif
