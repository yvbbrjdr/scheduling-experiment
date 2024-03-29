#define _GNU_SOURCE
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>

struct dblbuf {
    size_t size;
    size_t index;
    double *buffer;
};

static struct dblbuf *dblbuf_init(void);
static void dblbuf_destroy(struct dblbuf *buf);
static void dblbuf_push(struct dblbuf *buf, double data);

static double start_time;
static struct dblbuf *start_buffer, *end_buffer;
int log_dumping;

void log_init()
{
    start_buffer = dblbuf_init();
    end_buffer = dblbuf_init();
    start_time = cur_time();
    log_dumping = 0;
}

void log_start()
{
    if (start_buffer)
        dblbuf_push(start_buffer, cur_time() - start_time);
}

void log_end()
{
    if (end_buffer)
        dblbuf_push(end_buffer, cur_time() - start_time);
}

void log_dump()
{
    log_dumping = 1;
    for (size_t i = 0; i < start_buffer->index; ++i)
        printf("s%lf\n", start_buffer->buffer[i]);
    for (size_t i = 0; i < end_buffer->index; ++i)
        printf("e%lf\n", end_buffer->buffer[i]);
}

void log_destroy()
{
    struct dblbuf *tmp = start_buffer;
    start_buffer = NULL;
    dblbuf_destroy(tmp);
    tmp = end_buffer;
    end_buffer = NULL;
    dblbuf_destroy(tmp);
}

void dummy_syscall()
{
    syscall(SYS_getpid);
}

struct dblbuf *dblbuf_init(void)
{
    struct dblbuf *buf = malloc(sizeof(*buf));
    buf->size = 40000000;
    buf->index = 0;
    buf->buffer = malloc(buf->size * sizeof(*buf->buffer));
    return buf;
}

void dblbuf_destroy(struct dblbuf *buf)
{
    free(buf->buffer);
    free(buf);
}

void dblbuf_push(struct dblbuf *buf, double data)
{
    if (buf->index == buf->size) {
        buf->size *= 2;
        buf->buffer = realloc(buf->buffer, buf->size * sizeof(*buf->buffer));
    }
    buf->buffer[buf->index++] = data;
}

double cur_time()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + t.tv_usec / 1000000.0;
}

// Source: https://stackoverflow.com/questions/1407786/
int pin_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int pin_zero() {
    return pin_to_core(0);
}

int pin_one() {
    return pin_to_core(1);
}

int pin_disallow_zero() {
    int num_cores = get_core_count();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    for(int i = 1; i < num_cores; i++)
        CPU_SET(i, &cpuset);

    pthread_t current_thread = pthread_self();
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int pin_random_core() {
   return pin_to_core(rand() % (get_core_count() - 1) + 1);
}

int get_core_count() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}
