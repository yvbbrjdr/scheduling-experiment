#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "blockingthread.h"
#include "semathread.h"
#include "epollthread.h"
#include "userspacescheduler.h"
#include "generatorthread.h"
#include "utils.h"

static void handler(int);

int main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage: %s <b|s|e|u> thread_num rate\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if(get_core_count() < 2) {
        printf("system needs at least two cores (one to pin generator thread)\n");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handler);
    size_t n = atoi(argv[2]);
    size_t rate = atoi(argv[3]);
    //setup generator thread, log, and barrier
    log_init();
    pthread_t gen_tid;
    pthread_barrier_t initial;

    if( (argv[1][0] == 'u') ) {
        if (pthread_barrier_init(&initial, NULL, 2) != 0) {
            fprintf(stderr, "pthread_barrier_init: failed\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (pthread_barrier_init(&initial, NULL, n + 1) != 0) {
            fprintf(stderr, "pthread_barrier_init: failed\n");
            exit(EXIT_FAILURE);
        }
    }
    

    struct thread_context *gen_ctx;
    gen_ctx = thread_context_init();
    long *gen_pc_addr = malloc(sizeof(long));
    gen_ctx->gen_pc_addr = gen_pc_addr;
    gen_ctx->gen_rate = rate;
    gen_ctx->init = &initial;
    pthread_create(&gen_tid, NULL, thread_generator, gen_ctx);

    if (argv[1][0] == 'b')
        run_blocking_threads(n, &initial, gen_pc_addr);
    else if (argv[1][0] == 's')
        run_sema_threads(n, &initial, gen_pc_addr);
    else if (argv[1][0] == 'e')
        run_epoll_threads(n, &initial, gen_pc_addr);
    else if (argv[1][0] == 'u')
        run_userspace_scheduler(n, &initial, gen_pc_addr);
    else
        exit(EXIT_FAILURE);
    return 0;
}

void handler(int signal)
{
    (void) signal;
    log_dump();
    log_destroy();
    exit(EXIT_SUCCESS);
}
