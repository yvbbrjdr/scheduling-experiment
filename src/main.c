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
    srand(time(NULL));
    if (argc != 5) {
        printf("usage: %s <b|s|e|u> <s|r|n> thread_num rate\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (get_core_count() < 2) {
        printf("system needs at least two cores (one to pin generator thread)\n");
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handler);
    size_t n = atoi(argv[3]);
    size_t rate = atoi(argv[4]);
    pthread_barrier_t initial;

    if (argv[1][0] == 'u') {
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

    int (*pin_func)();
    switch (argv[2][0]) {
    case 's':
        pin_func = pin_one;
        break;
    case 'r':
        pin_func = pin_random_core;
        break;
    case 'n':
        pin_func = pin_disallow_zero;
        break;
    default:
        puts("wrong arguments for pinning");
        exit(EXIT_FAILURE);
        break;
    }

    volatile long gen_pc = 0;
    log_init();
    run_generator_thread(&initial, rate, &gen_pc);

    switch (argv[1][0]) {
    case 'b':
        run_blocking_threads(n, &initial, &gen_pc, pin_func);
        break;
    case 's':
        run_sema_threads(n, &initial, &gen_pc, pin_func);
        break;
    case 'e':
        run_epoll_threads(n, &initial, &gen_pc, pin_func);
        break;
    case 'u':
        run_userspace_scheduler(n, &initial, &gen_pc, pin_func);
        break;
    default:
        puts("wrong arguments for type");
        exit(EXIT_FAILURE);
        break;
    }
    pthread_barrier_destroy(&initial);
    return 0;
}

void handler(int signal)
{
    (void) signal;
    log_dump();
    log_destroy();
    exit(EXIT_SUCCESS);
}
