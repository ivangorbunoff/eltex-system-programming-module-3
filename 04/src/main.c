#include <string.h>

#include "roles.h"
#include "ui.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        ui_show_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-p") == 0) {
        return run_producer();
    }

    if (strcmp(argv[1], "-c") == 0) {
        return run_consumer();
    }

    ui_show_usage(argv[0]);

    return 1;
}
