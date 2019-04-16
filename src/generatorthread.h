#ifndef GENERATOR_THREAD
#define GENERATOR_THREAD

#include <stddef.h>
#include <pthread.h>
#include <sched.h>

void run_generator_thread(cpu_set_t cpuset);
void *thread_generator(void *_ctx);

#endif //GENERATOR_THREAD