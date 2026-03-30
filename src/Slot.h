#ifndef ORG_SLOT_H_
#define ORG_SLOT_H_


enum {LOVE_FORBIDDEN = 255};

typedef unsigned char love_t;

struct Slot {
	love_t* loveTable;
	int duration;
	int start;
};

#endif