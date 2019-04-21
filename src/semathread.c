#include "semathread.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "threadcontext.h"
#include "utils.h"

void run_sema_threads(size_t n, pthread_barrier_t *initial, long *gen_pc_addr)
{
    disallow_core(0);
    pthread_t tids[n];
    struct thread_context *ctxs[n];
    if (pthread_barrier_init(initial, NULL, n + 1) != 0) {
        fprintf(stderr, "pthread_barrier_init: failed\n");
        exit(EXIT_FAILURE);
    }
    sem_t r_semas[n - 1];
    for (size_t i = 0; i < n - 1; ++i)
        if (sem_init(r_semas + i, 0, 0) != 0) {
            fprintf(stderr, "sem_init: failed\n");
            exit(EXIT_FAILURE);
        }
    sem_t w_semas[n - 1];
    for (size_t i = 0; i < n - 1; ++i)
        if (sem_init(w_semas + i, 0, 1) != 0) {
            fprintf(stderr, "sem_init: failed\n");
            exit(EXIT_FAILURE);
        }
    ctxs[0] = thread_context_init();
    ctxs[0]->next_r_sema = r_semas;
    ctxs[0]->next_w_sema = w_semas;
    ctxs[0]->init = initial;
    ctxs[0]->gen_pc_addr = gen_pc_addr;
    pthread_create(tids, NULL, thread_sema_head, ctxs[0]);
    for(size_t i = 1; i < n - 1; i++) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_r_sema = r_semas + i - 1;
        ctxs[i]->prev_w_sema = w_semas + i - 1;
        ctxs[i]->next_r_sema = r_semas + i;
        ctxs[i]->next_w_sema = w_semas + i;
        ctxs[i]->init = initial;
        pthread_create(tids + i, NULL, thread_sema, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_r_sema = r_semas + n - 2;
    ctxs[n - 1]->prev_w_sema = w_semas + n - 2;
    ctxs[n - 1]->init = initial;
    pthread_create(tids + n - 1, NULL, thread_sema_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        sem_destroy(r_semas + i);
        sem_destroy(w_semas + i);
    }
    pthread_barrier_destroy(initial);
}

void *thread_sema(void *_ctx)
{
    disallow_core(0);
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_prev_r_sema_down(ctx);
        thread_context_next_w_sema_down(ctx);
        thread_context_next_r_sema_up(ctx);
        thread_context_prev_w_sema_up(ctx);
    }
    return NULL;
}

void *thread_sema_head(void *_ctx)
{
    disallow_core(0);
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    long current_pc = *(ctx->gen_pc_addr);
    for (;;) {
        long next_pc = *(ctx->gen_pc_addr);
        if(next_pc > current_pc) {
            thread_context_next_w_sema_down(ctx);
            long diff = next_pc - current_pc;
            current_pc = *(ctx->gen_pc_addr);
            for(int i = 0; i < diff; i++) {
                thread_context_next_r_sema_up(ctx);
            }
            
        }
    }
    return NULL;
}

void *thread_sema_tail(void *_ctx)
{
    disallow_core(0);
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_prev_r_sema_down(ctx);
        log_end();
        thread_context_prev_w_sema_up(ctx);
    }
    return NULL;
}
