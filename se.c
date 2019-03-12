#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

static void run_blocking_threads(size_t n);
static void run_mutex_threads(size_t n);

static void *thread_blocking(void *_ctx);
static void *thread_blocking_head(void *_ctx);
static void *thread_blocking_tail(void *_ctx);

static void *thread_mutex(void *_ctx);
static void *thread_mutex_head(void *_ctx);
static void *thread_mutex_tail(void *_ctx);

static double cur_time();

struct thread_context {
    int prev_fd;
    int next_fd;
    sem_t *prev_sema;
    sem_t *next_sema;
    pthread_barrier_t *init;
};

static struct thread_context *thread_context_init(void);
static void thread_context_destroy(struct thread_context *ctx);
static int thread_context_blocking_read(struct thread_context *ctx);
static int thread_context_blocking_write(struct thread_context *ctx, unsigned char byte);
static void thread_context_sema_up(struct thread_context *ctx);
static void thread_context_sema_down(struct thread_context *ctx);
static void thread_context_wait_barrier(struct thread_context *ctx);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    run_mutex_threads(atoi(argv[1]));
    return 0;
}

void run_mutex_threads(size_t n)
{
    pthread_t tids[n];
    struct thread_context *ctxs[n];
    pthread_barrier_t initial;
    if (pthread_barrier_init(&initial, NULL, n) != 0) {
        fprintf(stderr, "pthread_barrier_init: failed\n");
        exit(EXIT_FAILURE);
    }
    sem_t semas[n - 1];
    for (size_t i = 0; i < n - 1; ++i)
        if (sem_init(semas + i, 0, 0) != 0) {
            fprintf(stderr, "sem_init: failed\n");
            exit(EXIT_FAILURE);
        }
    ctxs[0] = thread_context_init();
    ctxs[0]->next_sema = semas;
    ctxs[0]->init = &initial;
    pthread_create(tids, NULL, thread_mutex_head, ctxs[0]);
    for(size_t i = 1; i < n - 1; i++) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_sema = semas + i - 1;
        ctxs[i]->next_sema = semas + i;
        ctxs[i]->init = &initial;
        pthread_create(tids + i, NULL, thread_mutex, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_sema = semas + n - 2;
    ctxs[n - 1]->init = &initial;
    pthread_create(tids + n - 1, NULL, thread_mutex_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
    for (size_t i = 0; i < n - 1; ++i)
        sem_destroy(semas + i);
    pthread_barrier_destroy(&initial);
}

void run_blocking_threads(size_t n)
{
    pthread_t tids[n];
    struct thread_context *ctxs[n];
    pthread_barrier_t initial;
    if (pthread_barrier_init(&initial, NULL, n) != 0) {
        fprintf(stderr, "pthread_barrier_init: failed\n");
        exit(EXIT_FAILURE);
    }
    int pipefd[2 * (n - 1)];
    for (size_t i = 0; i < n - 1; ++i)
        if (pipe(pipefd + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    ctxs[0] = thread_context_init();
    ctxs[0]->next_fd = pipefd[1];
    ctxs[0]->init = &initial;
    pthread_create(tids, NULL, thread_blocking_head, ctxs[0]);
    for (size_t i = 1; i < n - 1; ++i) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_fd = pipefd[2 * (i - 1)];
        ctxs[i]->next_fd = pipefd[2 * i + 1];
        ctxs[i]->init = &initial;
        pthread_create(tids + i, NULL, thread_blocking, ctxs[i]);
    }
    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_fd = pipefd[2 * (n - 2)];
    ctxs[n - 1]->init = &initial;
    pthread_create(tids + n - 1, NULL, thread_blocking_tail, ctxs[n - 1]);
    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }
    for (size_t i = 0; i < n - 1; ++i) {
        close(pipefd[2 * i]);
        close(pipefd[2 * i + 1]);
    }
    pthread_barrier_destroy(&initial);
}

void *thread_blocking(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
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
    thread_context_wait_barrier(ctx);
    for (;;) {
        int res = thread_context_blocking_write(ctx, 0);
        if (res != 1) {
            return NULL;
        }
        printf("Starting time: %lf\n", cur_time());
    }
}

void *thread_blocking_tail(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        int res = thread_context_blocking_read(ctx);
        if (res == -1)
            return NULL;
        printf("Ending time: %lf\n", cur_time());
    }
}

void *thread_mutex(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_sema_down(ctx);
        thread_context_sema_up(ctx);
    }
    return NULL;
}

void *thread_mutex_head(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_sema_up(ctx);
        printf("Start time: %lf\n", cur_time());
    }
    return NULL;
}

void *thread_mutex_tail(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    thread_context_wait_barrier(ctx);
    for (;;) {
        thread_context_sema_down(ctx);
        printf("End time: %lf\n", cur_time());
    }
    return NULL;
}

double cur_time()
{
    clock_t time = clock();
    return (double) time / CLOCKS_PER_SEC;
}

struct thread_context *thread_context_init(void)
{
    struct thread_context *ctx = malloc(sizeof(*ctx));
    if (ctx == NULL)
        return NULL;
    ctx->prev_fd = -1;
    ctx->next_fd = -1;
    ctx->prev_sema = NULL;
    ctx->next_sema = NULL;
    ctx->init = NULL;
    return ctx;
}

void thread_context_destroy(struct thread_context *ctx)
{
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

void thread_context_sema_up(struct thread_context *ctx)
{
    sem_post(ctx->next_sema);
}

void thread_context_sema_down(struct thread_context *ctx)
{
    sem_wait(ctx->prev_sema);
}

void thread_context_wait_barrier(struct thread_context *ctx)
{
    pthread_barrier_wait(ctx->init);
}
