#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct thread_context {
    int prev_fd;
    int next_fd;
    pthread_mutex_t *prev_lock;
    pthread_mutex_t *next_lock;
};

static struct thread_context *thread_context_init(void);
static void thread_context_destroy(struct thread_context *ctx);
static int thread_context_blocking_read(struct thread_context *ctx);
static int thread_context_blocking_write(struct thread_context *ctx, unsigned char byte);

int main(void)
{
    return 0;
}

struct thread_context *thread_context_init(void)
{
    struct thread_context *ctx = malloc(sizeof(*ctx));
    ctx->prev_fd = -1;
    ctx->next_fd = -1;
    ctx->prev_lock = NULL;
    ctx->next_lock = NULL;
    return ctx;
}

void thread_context_destroy(struct thread_context *ctx)
{
    free(ctx);
}

int thread_context_blocking_read(struct thread_context *ctx)
{
    unsigned char data;
    return read(ctx->prev_fd, &data, sizeof(data)) == -1 ? -1 : data;
}

int thread_context_blocking_write(struct thread_context *ctx, unsigned char byte)
{
    return write(ctx->next_fd, &byte, sizeof(byte));
}
