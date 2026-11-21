#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "ipc.h"
#include "shared_data.h"

#define IPC_PERMISSIONS 0600

int ipc_create(IpcContext* context) {
    context->shared_memory_fd = shm_open(
        SHARED_MEMORY_NAME,
        O_CREAT | O_EXCL | O_RDWR,
        IPC_PERMISSIONS
    );

    if (context->shared_memory_fd == -1) {
        return -1;
    }

    if (ftruncate(context->shared_memory_fd, SHARED_MEMORY_SIZE) == -1) {
        close(context->shared_memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    context->memory_base = mmap(
        NULL,
        SHARED_MEMORY_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        context->shared_memory_fd,
        0
    );

    if (context->memory_base == MAP_FAILED) {
        close(context->shared_memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    context->semaphore = sem_open(
        SEMAPHORE_NAME,
        O_CREAT | O_EXCL,
        IPC_PERMISSIONS,
        0
    );

    if (context->semaphore == SEM_FAILED) {
        munmap(context->memory_base, SHARED_MEMORY_SIZE);
        close(context->shared_memory_fd);
        shm_unlink(SHARED_MEMORY_NAME);
        return -1;
    }

    return 0;
}

int ipc_open(IpcContext* context) {
    context->shared_memory_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0);

    if (context->shared_memory_fd == -1) {
        return -1;
    }

    context->semaphore = sem_open(SEMAPHORE_NAME, 0);

    if (context->semaphore == SEM_FAILED) {
        close(context->shared_memory_fd);
        return -1;
    }

    context->memory_base = mmap(
        NULL,
        SHARED_MEMORY_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        context->shared_memory_fd,
        0
    );

    if (context->memory_base == MAP_FAILED) {
        sem_close(context->semaphore);
        close(context->shared_memory_fd);
        return -1;
    }

    return 0;
}

int ipc_sem_lock(const IpcContext* context) {
    return sem_wait(context->semaphore);
}

int ipc_sem_unlock(const IpcContext* context) {
    return sem_post(context->semaphore);
}

int ipc_close(const IpcContext* context) {
    int unmap_result = munmap(context->memory_base, SHARED_MEMORY_SIZE);
    int memory_close_result = close(context->shared_memory_fd);
    int semaphore_close_result = sem_close(context->semaphore);

    if (unmap_result == -1 || memory_close_result == -1 || semaphore_close_result == -1) {
        return -1;
    }

    return 0;
}

int ipc_remove(void) {
    int semaphore_result = sem_unlink(SEMAPHORE_NAME);
    int memory_result = shm_unlink(SHARED_MEMORY_NAME);

    if (semaphore_result == -1 || memory_result == -1) {
        return -1;
    }

    return 0;
}