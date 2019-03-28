#include "utils.h"
#include <stdio.h>
#include <sys/time.h>

static double start_time;

static double cur_time();

void log_init()
{
    start_time = cur_time();
}

void log_start()
{
    printf("Start time: %lf\n", cur_time() - start_time);
}

void log_end()
{
    printf("End time: %lf\n", cur_time() - start_time);
}

void log_dump()
{

}

void log_destroy()
{

}

double cur_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}
