#include "generatorthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <math.h>
#include <pthread.h>
#include "threadcontext.h"
#include "utils.h"

pthread_t run_generator_thread(pthread_barrier_t *initial, long gen_rate, volatile long *gen_pc_addr)
{
    pthread_t gen_tid;
    struct thread_context *ctx = thread_context_init();
    ctx->init = initial;
    ctx->gen_rate = gen_rate;
    ctx->gen_pc_addr = gen_pc_addr;
    pthread_create(&gen_tid, NULL, thread_generator, ctx);
    return gen_tid;
}

void *thread_generator(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    pin_to_core(0);
    thread_context_wait_barrier(ctx);
    double prev_time, next_time, period;
    prev_time = cur_time();
    period = 1.0 / ctx->gen_rate;
    for (;;) {
        next_time = cur_time();
        double time_diff = next_time - prev_time;
        if (time_diff > period) {
            long num_of_packets = floor(time_diff / period);
            prev_time += num_of_packets * period;
            for (long i = 0; i < num_of_packets; ++i) {
                ++*ctx->gen_pc_addr;
                log_start();
            }
        }
    }
    return NULL;
}
