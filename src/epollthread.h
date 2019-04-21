#ifndef EPOLLTHREAD_H
#define EPOLLTHREAD_H

#include <stddef.h>
#include <pthread.h>

void run_epoll_threads(size_t n, pthread_barrier_t *initial, long *gen_pc_addr);

void *thread_epoll(void *_ctx);

#endif // EPOLLTHREAD_H
