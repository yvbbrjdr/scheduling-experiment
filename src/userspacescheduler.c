#include "userspacescheduler.h"
#include "utils.h"
#include <stdio.h>

void run_userspace_scheduler(size_t n, pthread_barrier_t *initial, long *gen_pc_addr)
{
    disallow_core(0);
    pthread_barrier_wait(initial);
    struct thread_context *ctxs[n];
    for (size_t i = 0; i < n; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->func = userspace_scheduler;
    }
    ctxs[0]->func = userspace_scheduler_head;
    ctxs[n - 1]->func = userspace_scheduler_tail;

    long current_pc = *gen_pc_addr;
    for (;;) {
        long next_pc = *gen_pc_addr;
        if(next_pc > current_pc) {
            long diff = next_pc - current_pc;
            current_pc = *gen_pc_addr;
            for(int i = 0; i < diff; i++) {
                for (size_t i = 0; i < n; ++i)
                ctxs[i]->func(ctxs[i]);
            }
        
        }
    }
        
}
        

void userspace_scheduler(struct thread_context *ctx)
{
    (void) ctx;
    dummy_syscall();
}

void userspace_scheduler_head(struct thread_context *ctx)
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
