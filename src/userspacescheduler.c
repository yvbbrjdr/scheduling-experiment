#include "userspacescheduler.h"
#include "utils.h"

void run_userspace_scheduler(size_t n)
{
    log_init();
    for (;;)
        for (size_t i = 0; i < n; ++i)
            if (i == 0) {
                log_start();
                dummy_syscall();
            } else if (i == n - 1) {
                dummy_syscall();
                log_end();
            } else {
                dummy_syscall();
            }
}
