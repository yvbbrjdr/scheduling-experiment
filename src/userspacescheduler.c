#include "userspacescheduler.h"
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

void run_userspace_scheduler(size_t n, pthread_barrier_t *initial, volatile long *gen_pc_addr, int (*pin_func)())
{
    pin_func();
    pthread_barrier_wait(initial);
    struct thread_context *ctxs[n];
    for (size_t i = 0; i < n; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->func = userspace_scheduler;
    }
    ctxs[n - 1]->func = userspace_scheduler_tail;
    long current_pc = 0;
    for (;;) {
        if (log_dumping)
            pause();
        long next_pc = *gen_pc_addr;
        long diff = next_pc - current_pc;
        if (diff > 0) {
            current_pc = next_pc;
            for (long i = 0; i < diff; ++i)
                for (size_t i = 0; i < n; ++i)
                    ctxs[i]->func(ctxs[i]);
        }
    }
}

void userspace_scheduler(struct thread_context *ctx)
{
    (void) ctx;
    dummy_syscall();
}

void userspace_scheduler_tail(struct thread_context *ctx)
{
    (void) ctx;
    dummy_syscall();
    log_end();
}
