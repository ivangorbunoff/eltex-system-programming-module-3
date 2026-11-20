#ifndef IPC_H
#define IPC_H

typedef struct {
    int shared_memory_id;
    int semaphore_id;
    unsigned char* memory_base;
} IpcContext;

int ipc_create(IpcContext* context);

int ipc_open(IpcContext* context);

int ipc_sem_lock(const IpcContext* context);

int ipc_sem_unlock(const IpcContext* context);

int ipc_detach_mem(const IpcContext* context);

int ipc_remove(const IpcContext* context);

#endif
