#ifndef USERSPACESCHEDULER_H
#define USERSPACESCHEDULER_H

#include <stddef.h>
#include "threadcontext.h"
#include <pthread.h>

void run_userspace_scheduler(size_t n, pthread_barrier_t *initial, volatile long *gen_pc_addr, int (*pin_func)());

void userspace_scheduler(struct thread_context *ctx);
void userspace_scheduler_tail(struct thread_context *ctx);

#endif // USERSPACESCHEDULER_H
