#include <unistd.h>

#include "ipc.h"
#include "roles.h"
#include "shared_data.h"
#include "ui.h"

#define CONSUMER_DELAY_SECONDS 1

static void find_min_and_max(const NumberBlock* block, int* min, int* max) {
    *min = block->numbers[0];
    *max = block->numbers[0];

    for (size_t index = 1; index < block->number_count; index++) {
        if (block->numbers[index] < *min) {
            *min = block->numbers[index];
        }

        if (block->numbers[index] > *max) {
            *max = block->numbers[index];
        }
    }
}

int run_consumer(void) {
    IpcContext context;
    pid_t consumer_pid = getpid();
    int exit_code = 0;

    if (ipc_open(&context) == -1) {
        ui_show_error("Не удалось открыть IPC-объекты");
        return 1;
    }

    ui_show_consumer_started(consumer_pid);

    while (1) {
        if (ipc_sem_lock(&context) == -1) {
            ui_show_error("Не удалось захватить семафор");
            exit_code = 1;
            break;
        }

        SharedDataHeader* header = (SharedDataHeader*) context.memory_base;

        NumberBlock* block = shared_data_find_unprocessed_block(context.memory_base);

        if (block == NULL) {
            int producer_finished = header->producer_finished;

            if (ipc_sem_unlock(&context) == -1) {
                ui_show_error("Не удалось освободить семафор");
                exit_code = 1;
                break;
            }

            if (producer_finished) {
                break;
            }

            sleep(CONSUMER_DELAY_SECONDS);
            continue;
        }

        size_t number_count = block->number_count;
        int min;
        int max;

        find_min_and_max(block, &min, &max);

        block->number_count = 0;

        if (ipc_sem_unlock(&context) == -1) {
            ui_show_error("Не удалось освободить семафор");
            exit_code = 1;
            break;
        }

        ui_show_processed_block(consumer_pid, number_count, min, max);

        sleep(CONSUMER_DELAY_SECONDS);
    }

    if (ipc_close(&context) == -1) {
        ui_show_error("Не удалось закрыть IPC-объекты");
        exit_code = 1;
    }

    ui_show_status("Потребитель завершён");

    return exit_code;
}
