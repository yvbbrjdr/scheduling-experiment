#include "generatorthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include "threadcontext.h"
#include "utils.h"
#include <time.h>

#define BILLION 1000000000L

void *thread_generator(void *_ctx) {
    struct thread_context *ctx = _ctx;
    pin_to_core(0);
    thread_context_wait_barrier(ctx);
    struct timespec current, next;
    clock_gettime(CLOCK_MONOTONIC, &current);
    long current_time = current.tv_sec * BILLION + current.tv_nsec;
    long ns_period = BILLION / ctx->gen_rate;
    for (;;) {
        clock_gettime(CLOCK_MONOTONIC, &next);
        long next_time = next.tv_sec * BILLION + next.tv_nsec;
        if((next_time - current_time) > ns_period) {
            clock_gettime(CLOCK_MONOTONIC, &current);
            current_time = current.tv_sec * BILLION + current.tv_nsec;
            log_start();
            ++*(ctx->gen_pc_addr);
        }
    }  


    return NULL;
}
