#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include <semaphore.h>
#include <pthread.h>

struct thread_context {
    int prev_fd;
    int next_fd;
    sem_t *prev_r_sema;
    sem_t *prev_w_sema;
    sem_t *next_r_sema;
    sem_t *next_w_sema;
    pthread_barrier_t *init;
};

struct thread_context *thread_context_init(void);
void thread_context_destroy(struct thread_context *ctx);
int thread_context_read(struct thread_context *ctx);
int thread_context_write(struct thread_context *ctx, unsigned char byte);
void thread_context_prev_r_sema_down(struct thread_context *ctx);
void thread_context_prev_w_sema_up(struct thread_context *ctx);
void thread_context_next_r_sema_up(struct thread_context *ctx);
void thread_context_next_w_sema_down(struct thread_context *ctx);
void thread_context_wait_barrier(struct thread_context *ctx);

#endif // THREADCONTEXT_H
