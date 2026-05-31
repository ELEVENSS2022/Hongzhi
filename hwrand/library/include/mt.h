#ifndef MT_H
#define MT_H

#include <stdint.h>


void init_mt19937(uint32_t seed);

uint32_t mt19937_random(void);

#endif