#ifndef MUTEXTHREAD_H
#define MUTEXTHREAD_H

#include <stddef.h>

void run_mutex_threads(size_t n);

void *thread_mutex(void *_ctx);
void *thread_mutex_head(void *_ctx);
void *thread_mutex_tail(void *_ctx);

#endif // MUTEXTHREAD_H
