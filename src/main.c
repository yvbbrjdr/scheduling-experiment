#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "blockingthread.h"
#include "mutexthread.h"
#include "epollthread.h"
#include "utils.h"

static void handler(int);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    signal(SIGINT, handler);
    run_epoll_threads(atoi(argv[1]));
    return 0;
}

void handler(int signal)
{
    log_dump();
    log_destroy();
    exit(EXIT_SUCCESS);
}
