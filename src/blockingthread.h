#ifndef BLOCKINGTHREAD_H
#define BLOCKINGTHREAD_H

#include <stddef.h>
#include <pthread.h>

void run_blocking_threads(size_t n, pthread_barrier_t *initial, long *gen_pc_addr);

void *thread_blocking(void *_ctx);
void *thread_blocking_head(void *_ctx);
void *thread_blocking_tail(void *_ctx);

#endif // BLOCKINGTHREAD_H
