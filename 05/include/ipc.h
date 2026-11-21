#ifndef IPC_H
#define IPC_H

#include <semaphore.h>

#define SHARED_MEMORY_NAME "/memory"
#define SEMAPHORE_NAME "/semaphore"

typedef struct {
    int shared_memory_fd;
    sem_t* semaphore;
    unsigned char* memory_base;
} IpcContext;

int ipc_create(IpcContext* context);

int ipc_open(IpcContext* context);

int ipc_sem_lock(const IpcContext* context);

int ipc_sem_unlock(const IpcContext* context);

int ipc_close(const IpcContext* context);

int ipc_remove(void);

#endif