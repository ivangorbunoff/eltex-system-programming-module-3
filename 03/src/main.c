#include "chat.h"
#include "ui.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        ui_show_usage(argv[0]);
        return 1;
    }

    return run_chat(argv[1]);
}
