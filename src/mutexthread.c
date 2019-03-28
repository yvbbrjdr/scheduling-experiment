#include "mutexthread.h"
#include <pthread.h>
#include <semaphore.h>
#include "threadcontext.h"
#include "utils.h"

void run_mutex_threads(size_t n)
{
    pthread_t tids[n];
    struct thread_context *ctxs[n];
    pthread_barrier_t initial;
    if (pthread_barrier_init(&initial, NULL, n) != 0) {
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
    ctxs[0]->init = &initial;
    pthread_create(tids, NULL, thread_mutex_head, ctxs[0]);
    for(size_t i = 1; i < n - 1; i++) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_r_sema = r_semas + i - 1;
        ctxs[i]->prev_w_sema = w_semas + i - 1;
        ctxs[i]->next_r_sema = r_semas + i;
        ctxs[i]->next_w_sema = w_semas + i;
        ctxs[i]->init = &initial;
        pthread_create(tids + i, NULL, thread_mutex, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_r_sema = r_semas + n - 2;
    ctxs[n - 1]->prev_w_sema = w_semas + n - 2;
    ctxs[n - 1]->init = &initial;
    pthread_create(tids + n - 1, NULL, thread_mutex_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        sem_destroy(r_semas + i);
        sem_destroy(w_semas + i);
    }
    pthread_barrier_destroy(&initial);
}

void *thread_mutex(void *_ctx)
{
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

void *thread_mutex_head(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    start_time = cur_time();
    for (;;) {
        thread_context_next_w_sema_down(ctx);
        thread_context_next_r_sema_up(ctx);
        log_start();
    }
    return NULL;
}

void *thread_mutex_tail(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_prev_r_sema_down(ctx);
        thread_context_prev_w_sema_up(ctx);
        log_end();
    }
    return NULL;
}
