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
    //run this generator thread on a specific cpuset
    //this should pin itself to one thread (given by ctx)

    struct thread_context *ctx = _ctx;
    int s;
    //write differently depending on the mode

    // cpu_set_t cpuset = ctx->cpuset;

    // s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    // if (s != 0) {
    //     handle_error_en(s, "pthread_setaffinity_np");
    // }
    log_init();

    thread_context_wait_barrier(ctx);
    if(ctx->gen_mode == FILE_DESCRIPTOR) {
        struct timespec current, next;
        clock_gettime(CLOCK_MONOTONIC, &current);
        long current_time = current.tv_sec * BILLION + current.tv_nsec;
        long ns_period = BILLION / ctx->gen_rate;
        for (;;) {
            clock_gettime(CLOCK_MONOTONIC, &next);
            long next_time = next.tv_sec * BILLION + next.tv_nsec;
            if((next_time - current_time) > ns_period) {
                long bytes_needed = (next_time - current_time) / ns_period;
                clock_gettime(CLOCK_MONOTONIC, &current);
                current_time = current.tv_sec * BILLION + current.tv_nsec;
                for(int i = 0; i < bytes_needed; i++) {
                    log_start();
                    thread_context_write(ctx, 0);
                }
            }
            
        }  
    } else if(ctx->gen_mode == BUFFER) {
        //TODO
    }

    return NULL;
}
