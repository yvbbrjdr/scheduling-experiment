#include "blockingthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "threadcontext.h"
#include "generatorthread.h"
#include "utils.h"

void run_blocking_threads(size_t n, pthread_barrier_t *initial, volatile long *gen_pc_addr, enum pin_mode p)
{
    pthread_t tids[n];
    struct thread_context *ctxs[n];

    int (*pin_func)();
    if (p == single) {
        // single core pin (1)
        pin_func = &pin_one;
        pin_one();
    } else if (p == randompin) {
        pin_func = &pin_random_core;
        pin_random_core();
        // random core pin (nonzero)
    } else if (p == nopin) {
        pin_func = &pin_disallow_zero;
        pin_disallow_zero();
    }

    int pipefd[2 * (n - 1)];
    for (size_t i = 0; i < n - 1; ++i)
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    ctxs[0] = thread_context_init();
    ctxs[0]->next_fd = pipefd[1];
    ctxs[0]->init = initial;
    ctxs[0]->gen_pc_addr = gen_pc_addr;
    ctxs[0]->pin_func = pin_func;
    pthread_create(tids, NULL, thread_blocking_head, ctxs[0]);
    for (size_t i = 1; i < n - 1; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_fd = pipefd[2 * (i - 1)];
        ctxs[i]->next_fd = pipefd[2 * i + 1];
        ctxs[i]->init = initial;
        ctxs[i]->pin_func = pin_func;
        pthread_create(tids + i, NULL, thread_blocking, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_fd = pipefd[2 * (n - 2)];
    ctxs[n - 1]->init = initial;
    ctxs[n - 1]->pin_func = pin_func;
    pthread_create(tids + n - 1, NULL, thread_blocking_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        close(pipefd[2 * i]);
        close(pipefd[2 * i + 1]);
    }
    pthread_barrier_destroy(initial);
}

void *thread_blocking(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    ctx->pin_func();
    thread_context_wait_barrier(ctx);
    for (;;) {
        int res = thread_context_read(ctx);
        if (res == -1)
            return NULL;
        res = thread_context_write(ctx, (unsigned char) res);
        if (res != 1)
            return NULL;
    }
}

void *thread_blocking_head(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    ctx->pin_func();
    thread_context_wait_barrier(ctx);
    long current_pc = 0;
    for (;;) {
        long next_pc = *(ctx->gen_pc_addr);
        long diff = next_pc - current_pc;
        if (diff > 0) {
            current_pc = next_pc;
            for (long i = 0; i < diff; ++i) {
                int res = thread_context_write(ctx, 0);
                if (res != 1)
                    return NULL;
            }
        }
    }
}

void *thread_blocking_tail(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    ctx->pin_func();
    thread_context_wait_barrier(ctx);
    for (;;) {
        int res = thread_context_read(ctx);
        if (res == -1)
            return NULL;
        log_end();
    }
}
