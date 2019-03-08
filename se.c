#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

static void run_blocking_threads(size_t n);
static void run_mutex_threads(size_t n);

static void *thread_blocking(void *_ctx);
static void *thread_blocking_head(void *_ctx);
static void *thread_blocking_tail(void *_ctx);

static void *thread_mutex(void *_ctx);
static void *thread_mutex_head(void *_ctx);
static void *thread_mutex_tail(void *_ctx);


struct thread_context {
    char* input;
    int prev_fd;
    int next_fd;
    pthread_mutex_t *prev_lock;
    pthread_mutex_t *next_lock;
    pthread_barrier_t *init; 
};

static struct thread_context *thread_context_init(void);
static void thread_context_destroy(struct thread_context *ctx);
static int thread_context_blocking_read(struct thread_context *ctx);
static int thread_context_blocking_write(struct thread_context *ctx, unsigned char byte);

static double cur_time();

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    run_mutex_threads(atoi(argv[1]));
    return 0;
}

void run_mutex_threads(size_t n) {
    pthread_t tids[n]; 
    struct thread_context *ctxs[n]; 

    pthread_barrier_t initial;
    int ret = pthread_barrier_init(&initial, NULL, n - 1);
    if(ret != 0) {
        printf("failed to init barrier");
    }
    
    pthread_mutex_t mutexes[n - 1];
    for(size_t i = 0; i < n - 1; i++) {
        if(pthread_mutex_init(&mutexes[i], NULL) != 0) {
        printf("failed to initialize mutex");
        }
    }

    char *input = malloc(sizeof(*input) * 20);

    ctxs[0] = thread_context_init();
    ctxs[0]->next_lock = &mutexes[0];
    ctxs[0]->input = input;
    ctxs[0]->init = &initial;

    pthread_create(tids, NULL, thread_mutex_head, ctxs[0]);

    for(size_t i = 1; i < n - 1; i++) {
        ctxs[i] = thread_context_init();
        ctxs[i]->prev_lock = &mutexes[i - 1];
        ctxs[i]->next_lock = &mutexes[i];
        ctxs[i]->input = input;
        ctxs[i]->init = &initial;
        pthread_create(tids + i, NULL, thread_mutex, ctxs[i]);
    } 

    ctxs[n - 1] = thread_context_init();
    ctxs[n - 1]->prev_lock = &mutexes[n - 2];
    ctxs[n - 1]->input = input;
    ctxs[n - 1]->init = &initial;

    pthread_create(tids + n - 1, NULL, thread_mutex_tail, ctxs[n - 1]);

    for (size_t i = 0; i < n; ++i) {
        pthread_join(tids[i], NULL);
        thread_context_destroy(ctxs[i]);
    }

    
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

void *thread_mutex(void *_ctx) {
    struct thread_context *ctx = _ctx;
    int res = pthread_mutex_lock(ctx->next_lock);
    if(res != 0) {
        return NULL;
    }
    printf("waiting on barrier\n");
    pthread_barrier_wait(ctx->init);
    printf("waiting on mutex"); 
    res = pthread_mutex_lock(ctx->prev_lock);
    if(res != 0) {
        printf("error acquiring lock\n");
        printf("%d\n", &res);
    }
    printf("secured prev");
    res = pthread_mutex_unlock(ctx->prev_lock);

    res = pthread_mutex_unlock(ctx->next_lock);
}

void *thread_mutex_head(void *_ctx) {
    struct thread_context *ctx = _ctx; 
    int res;

    res = pthread_mutex_lock(ctx->next_lock);
    printf("secured head waiting on barrier\n");
    pthread_barrier_wait(ctx->init);
    
    *(ctx->input) = 2;
    //fgets(ctx->input, 20, stdin);
    printf("Start time: %f\n", cur_time());
    
    res = pthread_mutex_unlock(ctx->next_lock);
    printf("unlocked head mutex");
}

void *thread_mutex_tail(void *_ctx) {
    struct thread_context *ctx = _ctx;
    int res;
    res = pthread_mutex_lock(ctx->prev_lock);

    printf("End time: %f\n", cur_time());
    printf("%s", ctx->input);

    res = pthread_mutex_unlock(ctx->prev_lock);
}

double cur_time() {
    clock_t time = clock();
    return (double) time / CLOCKS_PER_SEC;
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
