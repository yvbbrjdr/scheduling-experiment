#ifndef USERSPACESCHEDULER_H
#define USERSPACESCHEDULER_H

#include <stddef.h>
#include "threadcontext.h"

void run_userspace_scheduler(size_t n);

void userspace_scheduler(struct thread_context *ctx);
void userspace_scheduler_head(struct thread_context *ctx);
void userspace_scheduler_tail(struct thread_context *ctx);

#endif // USERSPACESCHEDULER_H
