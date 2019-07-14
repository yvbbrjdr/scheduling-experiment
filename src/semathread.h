#ifndef SEMATHREAD_H
#define SEMATHREAD_H

#include <stddef.h>
#include <pthread.h>

void run_sema_threads(size_t n, pthread_barrier_t *initial, volatile long *gen_pc_addr);

void *thread_sema(void *_ctx);
void *thread_sema_head(void *_ctx);
void *thread_sema_tail(void *_ctx);

#endif // SEMATHREAD_H
