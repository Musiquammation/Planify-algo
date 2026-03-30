#include "algo.h"

#include "Task.h"
#include "Slot.h"
#include "shared.h"

#include "Array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


data_t data;


int* compareOptions_scores;

int compareOptions(const void* a, const void* b) {
	int s1 = compareOptions_scores[*(int*)a];
	int s2 = compareOptions_scores[*(int*)b];
	if (s2 > s1) return 1;
	if (s2 < s1) return -1;

	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

Layer* newLayers(void) {
	Layer* layers = malloc(sizeof(Layer) * shared.slots_len);
	for (int s = 0; s < shared.slots_len; s++) {
		int* options = malloc(sizeof(int) * shared.tasks_len);
		int* scores = malloc(sizeof(int) * shared.tasks_len);
		
		Slot slot = shared.slots[s];
		int endTime = slot.start + slot.duration;

		// Fill options
		int count = 0;
		for (int t = 0; t < shared.tasks_len; t++) {
			love_t love = slot.loveTable[shared.tasks[t].type];
			if (love > 250 || shared.tasks[t].bornline > slot.start)
				continue;
			
			int taskDuration = shared.tasks[t].duration;
			if (taskDuration > slot.duration || shared.tasks[t].deadline < endTime)
				continue;
			
			options[count] = t;
			
			int taskScore = taskDuration * love * shared.tasks[t].level;
			scores[t] = taskScore;
			
			count++;
		}

		compareOptions_scores = scores;
		qsort(options, count, sizeof(int), compareOptions);
	

		Layer* layer = &layers[s];
		layer->options = options;
		layer->scores = scores;
		layer->score = 0;
		layer->fullDuration = slot.duration;
		layer->optionCount = count;
	}

	return layers;
}

void freeLayers(Layer* layers) {
	Array_for(Layer, layers, shared.slots_len, l) {
		free(l->options);
		free(l->scores);
	}
	free(layers);
}




void fillCombination(
	int scoreBase,
	int leftDuration,
	Layer* layer
) {
	int* const options = layer->options;
	int layerScore = scoreBase;

	int subDuration = leftDuration;

	// First level
	int length = 0;
	int* positions = malloc(data.currentOptionCount * sizeof(int));

	for (int i = 0; i < data.currentOptionCount; i++) {
		int t = options[i];
		
		int optDuration = data.units[t].duration;

		if (optDuration <= subDuration && data.useCombin[t] == 0) {
			data.useCombin[t] = 1;
			positions[length] = t;
			length++;
			subDuration -= optDuration;
			layerScore += layer->scores[t];
		}
	}




	if (layerScore > data.bestCombinScore) {
		memcpy(data.useBestCombin, data.useCombin, shared.tasks_len);
		data.bestCombinScore = layerScore;
	}

	if (length == 0) {
		free(positions);
		return;
	}

	
	
	// Generate combinations
	int subScore = layerScore;
	
	while (true) {
		if (*shared.interrupt)
			goto exit;

		fillCombination(subScore, subDuration, layer);

		// Generate next combination
		int carry = 1;
		for (int i = length - 1; i >= 0; i--) {
			if (carry) {
				int task = positions[i];
				char c = data.useCombin[task];

				if (c == -1) {
					// Add task
					data.useCombin[task] = 1;
					subDuration -= data.units[task].duration;
					subScore += layer->scores[task];
					carry = 1;
				} else if (c == 1) {
					// Remove task
					data.useCombin[task] = -1;
					subDuration += data.units[task].duration;
					subScore -= layer->scores[task];
					carry = 0;
				}
			}
		}

		// Final overflow
		if (carry)
			break;
	}

	// Reset data
	for (int i = 0; i < length; i++) {
		data.useCombin[positions[i]] = 0;
	}


	exit:
	free(positions);
}






int pushLayers(int* usages, const int* layerDurations) {
	// Fill completion base
	int scoreBase = 0;
	Array* conflictLayers = malloc(shared.tasks_len * sizeof(Array)); // types: int to layer
	Array conflictTasks; // type: int
	Array_create(&conflictTasks, sizeof(int));

	// Fill useCombinPattern
	for (int i = 0; i < shared.tasks_len; i++) {
		data.useCombinPattern[i] = usages[i] == AVAILABLE ? 0 : -9;
	}



	int* subLayerDurations = malloc(shared.tasks_len * sizeof(int));
	int* easyTaken = malloc(shared.tasks_len * sizeof(int));
	for (int i = 0; i < shared.tasks_len; i++) {
		easyTaken[i] = -1;
	}

	
	for (int layerIndex = 0; layerIndex < shared.slots_len; layerIndex++) {
		Layer* layer = &data.layers[layerIndex];
		// Fill taken units and usageList
		data.bestCombinScore = -1;
		data.currentOptionCount = data.layers[layerIndex].optionCount;
		memcpy(data.useCombin, data.useCombinPattern, shared.tasks_len);

		int leftDuration = layerDurations[layerIndex];
		int realLeftDuration = leftDuration;
		fillCombination(0, leftDuration, layer);
		
		int* const options = layer->options;


		for (int i = 0; i < data.currentOptionCount; i++) {
			int t = options[i];
			if (data.useBestCombin[t] <= 0 || data.forbiddenList[t]) {
				continue;
			}

			int usage = usages[t];
			int optDuration = data.units[t].duration;

			if (usage == AVAILABLE) {
				// Consume task
				realLeftDuration -= optDuration;
				usages[t] = layerIndex;
				scoreBase += layer->scores[t];
				easyTaken[t] = layerIndex;
				
			} else if (usage >= 0) {
				// Create conflict
				usages[t] = CONFLICT;
				easyTaken[t] = -1;

				Array* c = &conflictLayers[t];
				Array_createAllowed(c, sizeof(int), 2);
				*Array_push(int, c) = usage;
				*Array_push(int, c) = layerIndex;
				*Array_push(int, &conflictTasks) = t;
				
			} else {
				*Array_push(int, &conflictLayers[t]) = layerIndex;
			}

			leftDuration -= optDuration;

			if (leftDuration == 0) {
				break; // end
			}

			
		}

		subLayerDurations[layerIndex] = realLeftDuration;
	}



	// No conflicts
	if (conflictTasks.length == 0) {
		free(easyTaken);
		free(conflictLayers);
		free(subLayerDurations);
		return scoreBase;
	}

	// Add conflicts to forbidden list
	Array_loop(int, conflictTasks, t) {
		data.forbiddenList[*t] = 1;
		printf("%d[%d] ", *t, conflictLayers[*t].length);
	}


	
	

	// Set first states
	Array_loop(int, conflictTasks, ptr) {
		int t = *ptr;
		int layer = *Array_get(int, conflictLayers[t], 0);
		usages[t] = layer;
	}

	int* states = calloc(conflictTasks.length, sizeof(int));


	int bestScore = 0x80000000;
	size_t size = sizeof(int) * shared.tasks_len;
	int* const subUsages = malloc(size);
	memcpy(subUsages, usages, size);
	int* const bestUsages = malloc(size);
	
	bool willFinish = false;
	
	for (int t = 0; t < shared.tasks_len; t++) {
		int l = easyTaken[t];
		if (l >= 0) {
			subUsages[t] = AVAILABLE;
			subLayerDurations[l] += shared.tasks[t].duration;
			scoreBase -= data.layers[l].scores[t];
		}
	}

	int* const givenSubUsages = malloc(size);

	
	

	
	while (true) {
		if (*shared.interrupt)
			goto exit;
			
		memcpy(givenSubUsages, subUsages, size);


		int addScore = pushLayers(givenSubUsages, subLayerDurations);
		int s = scoreBase + addScore;

		if (s > bestScore) {
			bestScore = s;
			memcpy(bestUsages, givenSubUsages, size);
		}





		// Move state
		int i = conflictTasks.length - 1; 
		while (true) {
			int s = states[i];
			int t = *Array_get(int, conflictTasks, i);
			int duration = shared.tasks[t].duration;			
			int l;

			if (s == -1) {
				s = 0;
				goto addTask;
			}
			
			l = *Array_get(int, conflictLayers[t], s);

			// Remove task
			subLayerDurations[l] += duration;
			scoreBase -= data.layers[l].scores[t];
			
			s++;
			if (s == conflictLayers[t].length) {
				// Ghost task
				subUsages[t] = -2;
				states[i] = -1;
				break;
			}

			
			// Add task
			
			addTask:
			l = *Array_get(int, conflictLayers[t], s);

			subLayerDurations[l] -= duration;
			scoreBase += data.layers[l].scores[t];

			subUsages[t] = l;
			states[i] = s;

			if (s > 0) {
				break;
			}

			i--;
			if (i < 0) {
				willFinish = true;
				goto exit;
			}

		}
	}

	exit:




	// Return best usages
	memcpy(usages, bestUsages, sizeof(int) * shared.tasks_len);

	free(subUsages);
	free(bestUsages);



	// Free data
	free(states);
	free(subLayerDurations);
	Array_loop(int, conflictTasks, ptr) {
		int t = *ptr;
		data.forbiddenList[t] = 0;
		Array_free(conflictLayers[t]);
	}

	Array_free(conflictTasks);
	free(conflictLayers);
	free(easyTaken);
	free(givenSubUsages);

	return bestScore;
}





int* runAlgo(void) {
	data.units = malloc(sizeof(Unit) * shared.tasks_len);
	for (int i = 0; i < shared.tasks_len; i++) {
		Unit* u = &data.units[i];
		const Task* t = &shared.tasks[i];
		u->task = t;
		u->duration = t->duration;
		u->type = t->type;
		u->level = t->level;
	}

	data.useBestCombin = malloc(shared.tasks_len);
	data.useCombin = malloc(shared.tasks_len);
	data.useCombinPattern = malloc(shared.tasks_len);
	data.layers = newLayers();
	data.forbiddenList = calloc(shared.tasks_len, 1);

	int* ownerListArg = malloc(sizeof(int) * shared.tasks_len);
	int* layerDurations = malloc(sizeof(int) * shared.slots_len);
	for (int i = 0; i < shared.tasks_len; i++) {
		ownerListArg[i] = AVAILABLE;
	}

	for (int i = 0; i < shared.slots_len; i++) {
		layerDurations[i] = shared.slots[i].duration;
	}

	int finalScore = pushLayers(ownerListArg, layerDurations);

	
	free(layerDurations);
	free(data.useCombinPattern);
	free(data.useCombin);
	free(data.useBestCombin);
	freeLayers(data.layers);
	free(data.units);
	free(data.forbiddenList);

	return ownerListArg;
}


