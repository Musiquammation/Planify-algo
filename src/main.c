#include <stdio.h>
#include <stdlib.h>


#include "Slot.h"
#include "Task.h"
#include "shared.h"
#include "algo.h"

int main() {
	int interrupt = 0;
	shared.interrupt = &interrupt;


	love_t math_l[] =  {200, 50};
	love_t info_l[] =  {50, 200};

	Slot slots[] = {
		{math_l, 6,  0}, // Lu mat {00-02}
		{math_l, 6,  4}, // Lu ap  {04-06}

		{info_l, 2, 10}, // Ma mat {10-12}
		{info_l, 2, 14}, // Ma ap  {14-16}

		// {math_l, 6, 20}, // Me mat {20-22}
		// {math_l, 2, 24}, // Me ap  {24-26}
	};

	Task tasks[] = {
		{"Gbr0", 1, 0, 1, -1, 0x7fffffff},
		{"Gbr1", 1, 0, 1, -1, 0x7fffffff},

		{"Ana0", 1, 0, 1, -1, 0x7fffffff},
		{"Ana1", 1, 0, 1, -1, 0x7fffffff},

		{"Prb0", 1, 0, 1, -1, 0x7fffffff},
		{"Prb1", 1, 0, 1, -1, 0x7fffffff},

		{"Inf0", 1, 1, 1, -1, 0x7fffffff},
		{"Inf1", 1, 1, 1, -1, 0x7fffffff},
		{"Inf2", 2, 1, 1, -1, 0x7fffffff},
	};

	shared.slots = slots;
	shared.tasks = tasks;

	shared.slots_len = sizeof(slots)/sizeof(Slot);
	shared.tasks_len = sizeof(tasks)/sizeof(Task);
	
	int* results = runAlgo();

	printf("\n\nResults:\n");
	for (int i = 0; i < shared.tasks_len; i++)
		printf("task[%d] -> slot[%d]\n", i, results[i]);

	free(results);

	return 0;
}