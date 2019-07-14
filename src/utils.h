#ifndef UTILS_H
#define UTILS_H

void log_init();
void log_start();
void log_end();
void log_dump();
void log_destroy();

double cur_time();

void dummy_syscall();

int pin_one();
int pin_zero();
int pin_disallow_zero();
int pin_random_core();

int get_core_count();

#endif // UTILS_H
