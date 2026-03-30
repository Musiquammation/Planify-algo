#ifndef ORG_ALGO_H_
#define ORG_ALGO_H_

#include "declarations.h"


enum {
	CONFLICT = -1,
	AVAILABLE = -2,
};

typedef struct {
	const Task* task;
	int duration;
	int type;
	int level;
} Unit;


typedef struct {
	int* options;
	int* scores;
	int fullDuration;
	int score;
	int optionCount;
} Layer;


typedef struct {
	Unit* units;
	Layer* layers;

	int currentOptionCount;
	int bestCombinScore;
	char* useCombin;
	char* useCombinPattern;
	char* useBestCombin;
	char* forbiddenList;
} data_t;


extern data_t data;


int* runAlgo(void);


#endif

