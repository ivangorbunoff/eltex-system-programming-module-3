#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "ui.h"
#include "transfer.h"

#define BUFFER_SIZE 4096

typedef enum {
    MESSAGE_READY,
    MESSAGE_FILE,
    MESSAGE_FINISH
} MessageType;

typedef struct {
    char file_name[PATH_MAX];
    off_t file_size;
} FileInfo;

static int write_all(int fd, const void* buffer, size_t size) {
    const char* bytes = buffer;
    size_t total_written = 0;

    while (total_written < size) {
        ssize_t written = write(fd, bytes + total_written, size - total_written);

        if (written == -1) {
            return -1;
        }

        total_written += (size_t) written;
    }

    return 0;
}

static int read_all(int fd, void* buffer, size_t size) {
    char* bytes = buffer;
    size_t total_read = 0;

    while (total_read < size) {
        ssize_t bytes_read = read(fd, bytes + total_read, size - total_read);

        if (bytes_read == -1) {
            return -1;
        }

        if (bytes_read == 0) {
            errno = EIO;
            return -1;
        }

        total_read += (size_t) bytes_read;
    }

    return 0;
}

static int send_file(ChannelEndpoint* endpoint, int file_fd, const char* file_name, off_t file_size) {
    FileInfo file_info = {0};

    int name_length = snprintf(file_info.file_name, sizeof(file_info.file_name), "%s", file_name);

    if (name_length < 0 || (size_t) name_length >= sizeof(file_info.file_name)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    file_info.file_size = file_size;

    MessageType message = MESSAGE_FILE;

    if (write_all(endpoint->write_fd, &message, sizeof(message)) == -1) {
        return -1;
    }

    if (write_all(endpoint->write_fd, &file_info, sizeof(file_info)) == -1) {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    off_t remaining = file_size;

    while (remaining > 0) {
        size_t block_size = BUFFER_SIZE;

        if (remaining < BUFFER_SIZE) {
            block_size = (size_t) remaining;
        }

        ssize_t bytes_read = read(file_fd, buffer, block_size);

        if (bytes_read == -1) {
            return -1;
        }

        if (bytes_read == 0) {
            errno = EIO;
            return -1;
        }

        if (write_all(endpoint->write_fd, buffer, (size_t) bytes_read) == -1) {
            return -1;
        }

        remaining -= bytes_read;
    }

    return 0;
}

int parent_send_files(ChannelEndpoint* endpoint, char* file_names[], int file_count) {
    MessageType message;

    if (read_all(endpoint->read_fd, &message, sizeof(message)) == -1) {
        ui_show_error("Не удалось получить READY");
        return -1;
    }

    if (message != MESSAGE_READY) {
        errno = EIO;
        ui_show_error("Получено неверное сообщение");
        return -1;
    }

    int result = 0;

    for (int i = 0; i < file_count; i++) {
        int file_fd = open(file_names[i], O_RDONLY);

        if (file_fd == -1) {
            ui_show_error(file_names[i]);
            result = -1;
            continue;
        }

        struct stat file_stat;

        if (fstat(file_fd, &file_stat) == -1) {
            ui_show_error(file_names[i]);
            close(file_fd);
            result = -1;
            continue;
        }

        if (send_file(endpoint, file_fd, file_names[i], file_stat.st_size) == -1) {
            ui_show_error(file_names[i]);
            close(file_fd);
            return -1;
        }

        close(file_fd);
    }

    message = MESSAGE_FINISH;

    if (write_all(endpoint->write_fd, &message, sizeof(message)) == -1) {
        ui_show_error("Не удалось отправить FINISH");
        return -1;
    }

    return result;
}

static int receive_file(ChannelEndpoint* endpoint) {
    FileInfo file_info;

    if (read_all(endpoint->read_fd, &file_info, sizeof(file_info)) == -1) {
        return -1;
    }

    char copy_name[PATH_MAX];

    int name_length = snprintf(copy_name, sizeof(copy_name), "%s.copy", file_info.file_name);

    if (name_length < 0 || (size_t) name_length >= sizeof(copy_name)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    int copy_fd = open(copy_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (copy_fd == -1) {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    off_t remaining = file_info.file_size;

    while (remaining > 0) {
        size_t block_size = BUFFER_SIZE;

        if (remaining < BUFFER_SIZE) {
            block_size = (size_t) remaining;
        }

        ssize_t bytes_read = read(endpoint->read_fd, buffer, block_size);

        if (bytes_read == -1) {
            close(copy_fd);
            return -1;
        }

        if (bytes_read == 0) {
            errno = EIO;
            close(copy_fd);
            return -1;
        }

        if (write_all(copy_fd, buffer, (size_t) bytes_read) == -1) {
            close(copy_fd);
            return -1;
        }

        remaining -= bytes_read;
    }

    close(copy_fd);
    ui_show_copied(file_info.file_name, file_info.file_size);

    return 0;
}

int child_receive_files(ChannelEndpoint* endpoint) {
    MessageType message = MESSAGE_READY;

    if (write_all(endpoint->write_fd, &message, sizeof(message)) == -1) {
        ui_show_error("Не удалось отправить READY");
        return -1;
    }

    while (1) {
        if (read_all(endpoint->read_fd, &message, sizeof(message)) == -1) {
            ui_show_error("Не удалось получить сообщение");
            return -1;
        }

        if (message == MESSAGE_FINISH) {
            return 0;
        }

        if (message != MESSAGE_FILE) {
            errno = EIO;
            ui_show_error("Получено неверное сообщение");
            return -1;
        }

        if (receive_file(endpoint) == -1) {
            ui_show_error("Не удалось принять файл");
            return -1;
        }
    }
}