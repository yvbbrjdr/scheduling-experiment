#ifndef GENERATORTHREAD_H
#define GENERATORTHREAD_H

#include <stddef.h>
#include <pthread.h>
#include <sched.h>
#include "threadcontext.h"

pthread_t run_generator_thread(pthread_barrier_t *initial, long gen_rate, volatile long *gen_pc_addr);
void *thread_generator(void *_ctx);

#endif //GENERATORTHREAD_H
