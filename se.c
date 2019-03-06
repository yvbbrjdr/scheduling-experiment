#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <linux/futex.h>
#include <sys/time.h>

static void run_blocking_threads(size_t n);

static void *thread_blocking(void *_ctx);
static void *thread_blocking_head(void *_ctx);
static void *thread_blocking_tail(void *_ctx);

static void *thread_mutex(void *_ctx, void *mutex);


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

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    run_blocking_threads(atoi(argv[1]));
    return 0;
}

void run_blocking_threads(size_t n)
{
    pthread_t tids[n];
    struct thread_context *ctxs[n];
    int pipefd[2];
    ctxs[0] = thread_context_init();
    ctxs[0]->prev_fd = STDIN_FILENO;
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    ctxs[0]->next_fd = pipefd[1];
    pthread_create(tids, NULL, thread_blocking_head, ctxs[0]);
    for (size_t i = 1; i < n - 1; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_fd = pipefd[0];
        if (pipe(pipefd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        ctxs[i]->next_fd = pipefd[1];
        pthread_create(tids + i, NULL, thread_blocking, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_fd = pipefd[0];
    ctxs[n - 1]->next_fd = STDOUT_FILENO;
    pthread_create(tids + n - 1, NULL, thread_blocking_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
}

void *thread_blocking(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    for (;;) {
        int res = thread_context_blocking_read(ctx);
        if (res == -1)
            return NULL;
        res = thread_context_blocking_write(ctx, (unsigned char) res);
        if (res != 1)
            return NULL;
    }
}

void *thread_blocking_head(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    for (;;) {
        int res = thread_context_blocking_read(ctx);
        if (res == -1)
            return NULL;
        res = thread_context_blocking_write(ctx, (unsigned char) res);
        //Print out start time
        clock_t start_t = clock();
        printf("Starting time: %f\n", (double) start_t / CLOCKS_PER_SEC);
        if (res != 1) {
            return NULL;
        }
    }
}

void *thread_blocking_tail(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    for (;;) {
        int res = thread_context_blocking_read(ctx);
        if (res == -1)
            return NULL;
        //Print out end time
        clock_t end_t = clock();
        printf("Ending time: %f\n", (double) end_t / CLOCKS_PER_SEC);
        res = thread_context_blocking_write(ctx, (unsigned char) res);
        if (res != 1)
            return NULL;
    }
}

void *thread_mutex(void *_ctx, void *mutex) 
{

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
    close(ctx->prev_fd);
    close(ctx->next_fd);
    if (ctx->prev_lock)
        pthread_mutex_destroy(ctx->prev_lock);
    if (ctx->next_lock)
        pthread_mutex_destroy(ctx->next_lock);
    free(ctx);
}

int thread_context_blocking_read(struct thread_context *ctx)
{
    unsigned char byte;
    return read(ctx->prev_fd, &byte, sizeof(byte)) <= 0 ? -1 : byte;
}

int thread_context_blocking_write(struct thread_context *ctx, unsigned char byte)
{
    return write(ctx->next_fd, &byte, sizeof(byte));
}
