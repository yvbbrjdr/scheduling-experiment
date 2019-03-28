#include "epollthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "blockingthread.h"
#include "threadcontext.h"
#include "utils.h"

void run_epoll_threads(size_t n)
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
        pthread_create(tids + i, NULL, thread_epoll, ctxs[i]);
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

void *thread_epoll(void *_ctx)
{
    struct thread_context *ctx = _ctx;
    int flags = fcntl(ctx->prev_fd, F_GETFL, 0);
    fcntl(ctx->prev_fd, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(ctx->next_fd, F_GETFL, 0);
    fcntl(ctx->next_fd, F_SETFL, flags | O_NONBLOCK);
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = ctx->prev_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ctx->prev_fd, &ev) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = ctx->next_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ctx->next_fd, &ev) == -1) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
    size_t que = 0;
    thread_context_wait_barrier(ctx);
    for (;;) {
        struct epoll_event evs[2];
        int n_ev = epoll_wait(epoll_fd, evs, 2, -1);
        if (n_ev == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < n_ev; ++i)
            if (evs[i].data.fd == ctx->prev_fd) {
                int res = thread_context_read_all(ctx);
                if (res < 0)
                    return NULL;
                que += res;
                for (; que > 0;) {
                    int res = thread_context_write(ctx, 0);
                    if (res >= 0)
                        que -= res;
                    else if (errno == EWOULDBLOCK)
                        break;
                    else
                        return NULL;
                }
            } else {
                for (; que > 0;) {
                    int res = thread_context_write(ctx, 0);
                    if (res >= 0)
                        que -= res;
                    else if (errno == EWOULDBLOCK)
                        break;
                    else
                        return NULL;
                }
            }
    }
}