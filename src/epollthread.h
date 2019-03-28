#ifndef EPOLLTHREAD_H
#define EPOLLTHREAD_H

#include <stddef.h>

void run_epoll_threads(size_t n);

void *thread_epoll(void *_ctx);

#endif // EPOLLTHREAD_H
