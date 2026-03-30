#include "api.h"

#include "algo.h"
#include "shared.h"
#include "Slot.h"
#include "Task.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#ifndef EMSCRIPTEN_KEEPALIVE
	#define EMSCRIPTEN_KEEPALIVE
#endif

// lecture int32 little-endian
static inline int32_t read_int32(const uint8_t* buf, size_t* offset) {
	int32_t v = 0;
	v |= (int32_t)buf[*offset + 0] << 0;
	v |= (int32_t)buf[*offset + 1] << 8;
	v |= (int32_t)buf[*offset + 2] << 16;
	v |= (int32_t)buf[*offset + 3] << 24;
	*offset += 4;
	return v;
}

// lecture byte
static inline uint8_t read_byte(const uint8_t* buf, size_t* offset) {
	return buf[(*offset)++];
}


void readBuffer(const uint8_t* buffer, size_t bufferSize) {
	size_t offset = 0;

	// ---- header ----
	int tasks_len  = read_int32(buffer, &offset);
	int slots_len  = read_int32(buffer, &offset);
	int types_len  = read_int32(buffer, &offset);

	// allocation des tableaux
	struct Slot* slots = calloc(slots_len, sizeof(struct Slot));
	struct Task* tasks = calloc(tasks_len, sizeof(struct Task));

	// ---- slots ----
	for (int i = 0; i < slots_len; i++) {
		int start    = read_int32(buffer, &offset);
		int duration = read_int32(buffer, &offset);

		slots[i].start = start;
		slots[i].duration = duration;
		slots[i].loveTable = calloc(types_len, sizeof(love_t));

		for (int t = 0; t < types_len; t++) {
			uint8_t scaled = read_byte(buffer, &offset);
			slots[i].loveTable[t] = scaled; // déjà entre 0 et 250
		}
	}

	// ---- tasks ----
	for (int i = 0; i < tasks_len; i++) {
		int duration  = read_int32(buffer, &offset);
		int typeIndex = read_int32(buffer, &offset);
		int bornline  = read_int32(buffer, &offset);
		int deadline  = read_int32(buffer, &offset);

		tasks[i].duration = duration;
		tasks[i].type     = typeIndex;
		tasks[i].name     = NULL; // non transmis dans le buffer
		tasks[i].level    = 1;    // idem
		tasks[i].bornline = bornline;
		tasks[i].deadline = deadline;
	}

	// mise à jour du shared global
	shared.tasks     = tasks;
	shared.slots     = slots;
	shared.tasks_len = tasks_len;
	shared.slots_len = slots_len;
}




__attribute__((used))
void apiSetInterruptPtr(volatile int* ptr) {
    shared.interrupt = ptr;
}

__attribute__((used))
int* apiRunAlgo(const uint8_t* buffer, int bufferSize) {
	readBuffer(buffer, bufferSize);
	return runAlgo();
}




