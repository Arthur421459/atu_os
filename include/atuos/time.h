#ifndef TIME_H
#define TIME_H
#include "lib/main.h"
typedef uint32_t nixt;
void sleep(uint32_t sec);
void usleep(uint32_t usec);
nixt get_nixt();
uint32_t get_exectime();
#endif