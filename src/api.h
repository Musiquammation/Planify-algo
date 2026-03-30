#ifndef ALGO_API_H_
#define ALGO_API_H_

#include "declarations.h"

#include <stddef.h>
#include <stdint.h>


void readBuffer(const uint8_t* buffer, size_t bufferSize);
void apiSetInterruptPtr(volatile int* ptr);
int* apiRunAlgo(const uint8_t* buffer, int bufferSize);

#endif

