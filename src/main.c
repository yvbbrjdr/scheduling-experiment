#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "blockingthread.h"
#include "semathread.h"
#include "epollthread.h"
#include "utils.h"

static void handler(int);

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s <b|s|e> thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handler);
    if (argv[1][0] == 'b')
        run_blocking_threads(atoi(argv[2]));
    else if (argv[1][0] == 's')
        run_sema_threads(atoi(argv[2]));
    else if (argv[1][0] == 'e')
        run_epoll_threads(atoi(argv[2]));
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
