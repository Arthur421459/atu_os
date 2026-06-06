#ifndef BUFFER_H
#define BUFFER_H
#include <stdint.h>
typedef struct {
    uint8_t buffer[128];
    int tail; // read
    int head; // write
} c_buffer;

#endif