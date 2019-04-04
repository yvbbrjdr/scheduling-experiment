#include "threadcontext.h"
#include <stdlib.h>
#include <unistd.h>

struct thread_context *thread_context_init(void)
{
    struct thread_context *ctx = malloc(sizeof(*ctx));
    if (ctx == NULL)
        return NULL;
    ctx->prev_fd = -1;
    ctx->next_fd = -1;
    ctx->prev_r_sema = NULL;
    ctx->prev_w_sema = NULL;
    ctx->next_r_sema = NULL;
    ctx->next_w_sema = NULL;
    ctx->init = NULL;
    ctx->func = NULL;
    return ctx;
}

void thread_context_destroy(struct thread_context *ctx)
{
    free(ctx);
}

int thread_context_read(struct thread_context *ctx)
{
    unsigned char byte;
    return read(ctx->prev_fd, &byte, sizeof(byte)) <= 0 ? -1 : byte;
}

int thread_context_read_all(struct thread_context *ctx)
{
    unsigned char byte[8192];
    return read(ctx->prev_fd, byte, sizeof(byte));
}

int thread_context_write(struct thread_context *ctx, unsigned char byte)
{
    return write(ctx->next_fd, &byte, sizeof(byte));
}

void thread_context_prev_r_sema_down(struct thread_context *ctx)
{
    sem_wait(ctx->prev_r_sema);
}

void thread_context_prev_w_sema_up(struct thread_context *ctx)
{
    sem_post(ctx->prev_w_sema);
}

void thread_context_next_r_sema_up(struct thread_context *ctx)
{
    sem_post(ctx->next_r_sema);
}

void thread_context_next_w_sema_down(struct thread_context *ctx)
{
    sem_wait(ctx->next_w_sema);
}

void thread_context_wait_barrier(struct thread_context *ctx)
{
    pthread_barrier_wait(ctx->init);
}
