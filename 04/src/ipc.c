#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "ipc.h"
#include "shared_data.h"

#define MEMORY_PROJECT_ID 'M'
#define SEMAPHORE_PROJECT_ID 'S'
#define IPC_PERMISSIONS 0600

union semun {
    int val;
};

static int get_ipc_keys(key_t* memory_key, key_t* semaphore_key) {
    *memory_key = ftok(".", MEMORY_PROJECT_ID);

    if (*memory_key == -1) {
        return -1;
    }

    *semaphore_key = ftok(".", SEMAPHORE_PROJECT_ID);

    if (*semaphore_key == -1) {
        return -1;
    }

    return 0;
}

int ipc_create(IpcContext* context) {
    key_t memory_key;
    key_t semaphore_key;

    if (get_ipc_keys(&memory_key, &semaphore_key) == -1) {
        return -1;
    }

    context->shared_memory_id = shmget(
        memory_key,
        SHARED_MEMORY_SIZE,
        IPC_CREAT | IPC_EXCL | IPC_PERMISSIONS
    );

    if (context->shared_memory_id == -1) {
        return -1;
    }

    void* attached_memory = shmat(context->shared_memory_id, NULL, 0);

    if (attached_memory == (void*) -1) {
        shmctl(context->shared_memory_id, IPC_RMID, NULL);
        return -1;
    }

    context->memory_base = attached_memory;

    context->semaphore_id = semget(
        semaphore_key,
        1,
        IPC_CREAT | IPC_EXCL | IPC_PERMISSIONS
    );

    if (context->semaphore_id == -1) {
        shmdt(context->memory_base);
        shmctl(context->shared_memory_id, IPC_RMID, NULL);
        return -1;
    }

    union semun semaphore_value;
    semaphore_value.val = 0;

    if (semctl(context->semaphore_id, 0, SETVAL, semaphore_value) == -1) {
        semctl(context->semaphore_id, 0, IPC_RMID);
        shmdt(context->memory_base);
        shmctl(context->shared_memory_id, IPC_RMID, NULL);
        return -1;
    }

    return 0;
}

int ipc_open(IpcContext* context) {
    key_t memory_key;
    key_t semaphore_key;

    if (get_ipc_keys(&memory_key, &semaphore_key) == -1) {
        return -1;
    }

    context->shared_memory_id = shmget(memory_key, SHARED_MEMORY_SIZE, 0);

    if (context->shared_memory_id == -1) {
        return -1;
    }

    context->semaphore_id = semget(semaphore_key, 1, 0);

    if (context->semaphore_id == -1) {
        return -1;
    }

    void* attached_memory = shmat(context->shared_memory_id, NULL, 0);

    if (attached_memory == (void*) -1) {
        return -1;
    }

    context->memory_base = attached_memory;

    return 0;
}

int ipc_sem_lock(const IpcContext* context) {
    struct sembuf lock_operation = {0, -1, 0};

    return semop(context->semaphore_id, &lock_operation, 1);
}

int ipc_sem_unlock(const IpcContext* context) {
    struct sembuf unlock_operations[2] = {
        {0, 0, 0},
        {0, 1, 0}
    };

    return semop(context->semaphore_id, unlock_operations, 2);
}

int ipc_detach_mem(const IpcContext* context) {
    return shmdt(context->memory_base);
}

int ipc_remove(const IpcContext* context) {
    int semaphore_result = semctl(context->semaphore_id, 0, IPC_RMID);
    int memory_result = shmctl(context->shared_memory_id, IPC_RMID, NULL);

    if (semaphore_result == -1 || memory_result == -1) {
        return -1;
    }

    return 0;
}