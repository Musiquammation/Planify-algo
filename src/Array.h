#ifndef TOOLS_ARRAY_H_
#define TOOLS_ARRAY_H_

#include <stdbool.h>
#include <stdlib.h>



#define Array_loop(Type, array, i)\
for (Type *i = (array).data, *const i##_end = i + (array).length; i < i##_end; i++)

#define Array_loopPtr(Type, array, i)\
for (Type **i = (array).data, **const i##_end = i + (array).length; i < i##_end; i++)

#define Array_for(Type, array, length, i)\
for (Type *i = (array), *const i##_end = i + length; i < i##_end; i++)

#define Array_forPtr(Type, array, length, i)\
for (Type **i = (array), **const i##_end = i + length; i < i##_end; i++)


#define Array_getPos(ptr, array) (((void*)(ptr) - (array).data) / (array).size)


typedef struct {
	void* data;
	size_t size;
	int length;
	int reserved;
} Array;

typedef int(Array_SortComparator_t)(const void* a, const void* b);

void Array_create(Array* array, size_t size);
void Array_createAllowed(Array* array, size_t size, int allowed);
#define Array_free(array) {if ((array).reserved) {free((array).data);};}

void Array_copy(const Array* src, Array* dest);
void* Array_pushFastArray(Array* array);
void* Array_pushSafeArray(Array* array);
void* Array_search(Array* array, const void* data, Array_SortComparator_t* comparator);
void* Array_binarySearch(const Array* array, const void* data, Array_SortComparator_t* comparator); // Array should be sorted
void* Array_binaryCompare(Array* array, const void* data, Array_SortComparator_t* comparator); // Array should be sorted
void* Array_pushSort(Array* array, const void* data, Array_SortComparator_t* comparator); // Array should be sorted
void* Array_tryDirectPush(Array* array);
void* Array_pushInEmpty(Array* array, bool(*isEmpty)(const void*));
void Array_shrinkToFit(Array* array);
void* Array_reach(Array* array, int index, const void* emptyValue);
#define Array_push(Type, arrayPtr) ((Type*)Array_pushSafeArray((arrayPtr)))

void* Array_getAt(Array* array, int index);
#define Array_get(Type, array, index) ((Type*)((array).data) + (index))


#endif