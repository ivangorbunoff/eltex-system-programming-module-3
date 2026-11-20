#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ipc.h"
#include "roles.h"
#include "shared_data.h"
#include "ui.h"

#define MIN_NUMBERS_PER_BLOCK 3
#define MAX_NUMBERS_PER_BLOCK 10

#define MIN_RANDOM_NUMBER -100
#define MAX_RANDOM_NUMBER 100

#define PRODUCER_DELAY_SECONDS 1

static size_t generate_numbers(int* numbers) {
    size_t number_count =
        MIN_NUMBERS_PER_BLOCK +
        (size_t) (rand() % (MAX_NUMBERS_PER_BLOCK - MIN_NUMBERS_PER_BLOCK + 1));

    for (size_t index = 0; index < number_count; index++) {
        numbers[index] =
            MIN_RANDOM_NUMBER +
            rand() % (MAX_RANDOM_NUMBER - MIN_RANDOM_NUMBER + 1);
    }

    return number_count;
}

static int cleanup_producer(IpcContext* context, int exit_code) {
    if (ipc_remove(context) == -1) {
        ui_show_error("Не удалось удалить IPC-объекты");
        exit_code = 1;
    }

    if (ipc_detach_mem(context) == -1) {
        ui_show_error("Не удалось отключить разделяемую память");
        exit_code = 1;
    }

    return exit_code;
}

int run_producer(void) {
    IpcContext context;
    int numbers[MAX_NUMBERS_PER_BLOCK];

    if (ipc_create(&context) == -1) {
        ui_show_error("Не удалось создать IPC-объекты");
        return 1;
    }

    shared_data_init(context.memory_base);

    if (ipc_sem_unlock(&context) == -1) {
        ui_show_error("Не удалось открыть доступ к памяти");
        return cleanup_producer(&context, 1);
    }

    ui_show_producer_started(context.shared_memory_id, context.semaphore_id);

    srand((unsigned) time(NULL));

    while (1) {
        size_t number_count = generate_numbers(numbers);

        if (ipc_sem_lock(&context) == -1) {
            ui_show_error("Не удалось захватить семафор");
            return cleanup_producer(&context, 1);
        }

        int block_added = shared_data_add_block(context.memory_base, numbers, number_count);

        if (!block_added) {
            SharedDataHeader* header = (SharedDataHeader*) context.memory_base;

            header->producer_finished = 1;

            if (ipc_sem_unlock(&context) == -1) {
                ui_show_error("Не удалось освободить семафор");
                return cleanup_producer(&context, 1);
            }

            break;
        }

        if (ipc_sem_unlock(&context) == -1) {
            ui_show_error("Не удалось освободить семафор");
            return cleanup_producer(&context, 1);
        }

        ui_show_generated_block(numbers, number_count);
        sleep(PRODUCER_DELAY_SECONDS);
    }

    ui_show_status("Разделяемая память заполнена");

    while (1) {
        if (ipc_sem_lock(&context) == -1) {
            ui_show_error("Не удалось захватить семафор");
            return cleanup_producer(&context, 1);
        }

        if (shared_data_all_blocks_processed(context.memory_base)) {
            ui_show_status("Все блоки обработаны");
            return cleanup_producer(&context, 0);
        }

        if (ipc_sem_unlock(&context) == -1) {
            ui_show_error("Не удалось освободить семафор");
            return cleanup_producer(&context, 1);
        }

        sleep(PRODUCER_DELAY_SECONDS);
    }
}
