#ifndef SEMATHREAD_H
#define SEMATHREAD_H

#include <stddef.h>

void run_sema_threads(size_t n, size_t rate);

void *thread_sema(void *_ctx);
void *thread_sema_head(void *_ctx);
void *thread_sema_tail(void *_ctx);

#endif // SEMATHREAD_H
