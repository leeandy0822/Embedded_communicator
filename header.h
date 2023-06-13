#ifndef HEADER_H
#define HEADER_H

#include <pthread.h>

extern pthread_mutex_t lcd_mutex;
extern pthread_barrier_t init;
extern struct LCD lcd;

#endif  // HEADER_H
