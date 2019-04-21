#include "userspacescheduler.h"
#include "utils.h"

void run_userspace_scheduler(size_t n, size_t rate)
{
    struct thread_context *ctxs[n];
    for (size_t i = 0; i < n; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->func = userspace_scheduler;
    }
    ctxs[0]->func = userspace_scheduler_head;
    ctxs[n - 1]->func = userspace_scheduler_tail;
    log_init();
    for (;;)
        for (size_t i = 0; i < n; ++i)
            ctxs[i]->func(ctxs[i]);
}

void userspace_scheduler(struct thread_context *ctx)
{
    (void) ctx;
    dummy_syscall();
}

void userspace_scheduler_head(struct thread_context *ctx)
{
    (void) ctx;
    log_start();
    dummy_syscall();
}

void userspace_scheduler_tail(struct thread_context *ctx)
{
    (void) ctx;
    dummy_syscall();
    log_end();
}
