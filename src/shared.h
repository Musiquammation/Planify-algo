#ifndef ORG_SHARED_H_
#define ORG_SHARED_H_

#include "declarations.h"
#include <stddef.h>


typedef struct {
	const Task* tasks;
	const Slot* slots;
	int tasks_len;
	int slots_len;
	volatile int* interrupt;
} shared_t;

extern shared_t shared;

#endif


