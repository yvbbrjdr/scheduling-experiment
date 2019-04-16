#include "generatorthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include "threadcontext.h"
#include "utils.h"
#include <time.h>

#define BILLION 1000000000

void *thread_generator(void *_ctx) {
    //run this generator thread on a specific cpuset
    //this should pin itself to one thread (given by ctx)

    struct thread_context *ctx = _ctx;
    int s;
    pthread_t thread;

    //write differently depending on the mode

    thread = pthread_self();

    cpu_set_t cpuset = ctx->cpuset;

    s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0) {
        handle_error_en(s, "pthread_setaffinity_np");
    }

    if(ctx->gen_mode == FILE_DESCRIPTOR) {
        //write to nextfd in a loop depending on if the time has advanced far enough
        struct timespec current, next;
        clock_gettime(CLOCK_MONOTONIC, &current);
        for (;;) {
            clock_gettime(CLOCK_MONOTONIC, &next);
            long bytes_needed = (next.tv_nsec - current.tv_nsec) * BILLION * ctx->rate;
            clock_gettime(CLOCK_MONOTONIC, &current);
            for(int i = 0; i < bytes_needed; i++) {
                log_start();
                thread_context_write(ctx->next_fd, 0);
            }
        }  
    } else if(ctx->gen_mode == BUFFER) {
        //TODO
    }
}