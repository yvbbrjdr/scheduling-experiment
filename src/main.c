#include <stdio.h>
#include <stdlib.h>
#include "blockingthread.h"
#include "mutexthread.h"
#include "epollthread.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s thread_num\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    run_epoll_threads(atoi(argv[1]));
    return 0;
}
