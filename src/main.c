#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "blockingthread.h"
#include "semathread.h"
#include "epollthread.h"
#include "userspacescheduler.h"
#include "utils.h"

static void handler(int);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s <b|s|e|u> thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handler);
    size_t n = atoi(argv[2]);
    if (argv[1][0] == 'b')
        run_blocking_threads(n);
    else if (argv[1][0] == 's')
        run_sema_threads(n);
    else if (argv[1][0] == 'e')
        run_epoll_threads(n);
    else if (argv[1][0] == 'u')
        run_userspace_scheduler(n);
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
