#ifndef BLOCKINGTHREAD_H
#define BLOCKINGTHREAD_H

#include <stddef.h>

void run_blocking_threads(size_t n);

void *thread_blocking(void *_ctx);
void *thread_blocking_head(void *_ctx);
void *thread_blocking_tail(void *_ctx);

#endif // BLOCKINGTHREAD_H
