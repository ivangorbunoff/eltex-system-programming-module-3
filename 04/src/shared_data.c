#include <string.h>

#include "shared_data.h"

static size_t align_up(size_t size, size_t alignment) {
    return (size + alignment - 1) / alignment * alignment;
}

static size_t get_aligned_block_size(size_t number_count) {
    size_t raw_size = sizeof(NumberBlock) + number_count * sizeof(int);

    return align_up(raw_size, _Alignof(NumberBlock));
}

void shared_data_init(unsigned char* memory_base) {
    SharedDataHeader* header = (SharedDataHeader*) memory_base;

    header->first_block_start = 0;
    header->last_block_start = 0;
    header->free_memory_start = align_up(sizeof(SharedDataHeader), _Alignof(NumberBlock));
    header->producer_finished = 0;
}

int shared_data_add_block(
    unsigned char* memory_base,
    const int* numbers,
    size_t number_count
) {
    SharedDataHeader* header = (SharedDataHeader*) memory_base;
    size_t block_size = get_aligned_block_size(number_count);
    size_t new_block_start = header->free_memory_start;

    if (new_block_start + block_size > SHARED_MEMORY_SIZE) {
        return 0;
    }

    NumberBlock* new_block = (NumberBlock*) (memory_base + new_block_start);
    new_block->number_count = number_count;
    new_block->next_block_start = 0;

    memcpy(new_block->numbers, numbers, number_count * sizeof(int));

    if (header->first_block_start == 0) {
        header->first_block_start = new_block_start;
    } else {
        NumberBlock* last_block = (NumberBlock*) (memory_base + header->last_block_start);
        last_block->next_block_start = new_block_start;
    }

    header->last_block_start = new_block_start;
    header->free_memory_start += block_size;

    return 1;
}

NumberBlock* shared_data_find_unprocessed_block(unsigned char* memory_base) {
    SharedDataHeader* header = (SharedDataHeader*) memory_base;
    size_t current_block_start = header->first_block_start;

    while (current_block_start != 0) {
        NumberBlock* current_block = (NumberBlock*) (memory_base + current_block_start);

        if (current_block->number_count > 0) {
            return current_block;
        }

        current_block_start = current_block->next_block_start;
    }

    return NULL;
}

int shared_data_all_blocks_processed(const unsigned char* memory_base) {
    const SharedDataHeader* header = (const SharedDataHeader*) memory_base;
    size_t current_block_start = header->first_block_start;

    while (current_block_start != 0) {
        const NumberBlock* current_block = (const NumberBlock*) (memory_base + current_block_start);

        if (current_block->number_count > 0) {
            return 0;
        }

        current_block_start = current_block->next_block_start;
    }

    return 1;
}
