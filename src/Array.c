#include "Array.h"
#include <stdlib.h>
#include <string.h>

void Array_create(Array* array, size_t size) {
	array->reserved = 0;
	array->length = 0;
	array->size = size;
}

void Array_createAllowed(Array* array, size_t size, int allowed) {
	array->reserved = allowed;
	array->length = 0;
	array->size = size;
	array->data = malloc(size * allowed);
}

void Array_copy(const Array* src, Array* dest) {
	const int length = src->length;
	const size_t size = src->size;

	dest->reserved = length;
	dest->length = length;
	dest->size = size;

	const int p = length * size;
	void* const data = malloc(p);
	dest->data = data;
	memcpy(dest->data, data, p);
}


void* Array_pushFastArray(Array* array) {
	void* ret = array->data + array->length * array->size;
	array->length++;
	return ret;
}

void* Array_pushSafeArray(Array* array) {
	const int length = array->length;
	const int reserved = array->reserved;

	// Enought reserved data
	if (length < reserved)
		return Array_pushFastArray(array);
	
	// Realloc
	const size_t size = array->size;
	void* newData;
	int nextReserved;
	
	if (reserved) {
		nextReserved = reserved<<1;
		newData = malloc(size * nextReserved);
		void* const oldData = array->data;
		memcpy(newData, oldData, reserved * size);
		free(oldData);
	} else {
		nextReserved = 2;
		newData = malloc(size * nextReserved);
	}

	array->data = newData;
	array->reserved = nextReserved;
	array->length++;

	return newData + length * size;
}

void* Array_tryDirectPush(Array* array) {
	if (array->length < array->reserved)
		return Array_pushFastArray(array);

	return NULL;	
}


void* Array_pushInEmpty(Array* array, bool(*isEmpty)(const void*)) {
	const size_t size = array->size;
	for (void* i = array->data, *const end = i + size * array->length; i != end; i += size)
		if ((*isEmpty)(i))
			return i;
	
	return Array_pushSafeArray(array);
}


void* Array_search(Array* array, const void* data, Array_SortComparator_t* comparator) {
	const size_t size = array->size;
	for (void* ptr = array->data, *end = ptr += array->length * size; ptr != end; ptr++) {
		if ((*comparator)(ptr, data) == 0)
			return ptr;
	}

	return NULL;
}


void* Array_binarySearch(const Array* array, const void* data, Array_SortComparator_t* comparator) {
	int max = array->length - 1;
	if (max == -1)
		return NULL; // array is empty

	int min = 0;

	const size_t size = array->size;
	void* const list = array->data;

	while (min < max) {
		const int i = ((max - min) >> 1) + min;

		int cmp = (*comparator)(list + size*i, data);
		if (cmp == 0)
			return list + size*i;
		
		if (cmp > 0) {
			max = i-1;
		} else {
			min = i+1;
		}
	}

	if ((*comparator)(list + size*min, data) == 0) {
		return list + size*min;
	}

	return NULL; // not found
}

void* Array_binaryCompare(Array* array, const void* data, Array_SortComparator_t* comparator) {
	int max = array->length - 1;
	if (max == -1)
		return NULL; // array is empty

	const size_t size = array->size;
	void* const list = array->data;

	// Only one element
	if (max == 0)
		return (*comparator)(list, data) >= 0 ? list : list+size;
	
	if ((*comparator)(list, data) >= 0)
		return list;


	if ((*comparator)(list + size*max, data) <= 0)
		return list + array->length * size; // last element


	int min = 0;
	while (true) {
		const int i = ((max - min) >> 1) + min;
		const int cmp = (*comparator)(list + size*i, data);

		if (cmp == 0)
			return list + size*i;
		
		if (cmp > 0) {
			max = i;
		} else {
			min = i;
		}

		if (max - min == 1) {
			return list + size*max;
		}
	}

}


void* Array_pushSort(Array* array, const void* data, Array_SortComparator_t* comparator) {
	const int length = array->length;

	const size_t size = array->size;
    
    // list is empty
	if (length == 0) {
		void* const ptr = Array_pushSafeArray(array);
		memcpy(ptr, data, size);
		return ptr;
	}

	// allow space for the new element
    Array_pushSafeArray(array);
    array->length--;
    

	void* const ptr = Array_binaryCompare(array, data, comparator);
	void* const list = array->data;
	memmove(ptr+size, ptr, list + length * size - ptr);
	memcpy(ptr, data, size);
    array->length++;
	
    return ptr;
}


void Array_shrinkToFit(Array* array) {
	const int length = array->length;
	array->reserved = length;

	const int total = length * array->size;
	void* const newData = malloc(total);
	void* const oldData = array->data;
	memcpy(newData, oldData, total);
	free(oldData);
}

void* Array_reach(Array* array, int index, const void* emptyValue) {
	const int length = array->length;

	if (length == index)
		return Array_pushSafeArray(array);
	
	if (index < length)
		return Array_getAt(array, index);
	
	// Here, index > length
	const int reserved = array->reserved;
	const size_t size = array->size;

	if (reserved == 0) {
		array->reserved = index+1;
		array->length = index+1;

		void* ptr = malloc(size * (index+1));
		array->data = ptr;

		void* const end = ptr + index * size;
		for (; ptr != end; ptr += size)
			memcpy(ptr, emptyValue, size);

		return ptr;
	}


	void* const data = array->data;
	if (index < reserved) {
		void* const aim = data + index * size;

		
		for (void* ptr = data + length * size; ptr != aim; ptr += size)
			memcpy(ptr, emptyValue, size);
		
		array->length = index+1;
		return aim;
	}

	void* const newData = malloc(size * (index+1));
	array->data = newData;
	array->reserved = index+1;
	array->length = index+1;

	// Copy old data
	memcpy(newData, data, size * length);
	
	// Fill with emptyValue
	for (void* ptr = newData + size * length, *const end = newData + index * size; ptr!= end; ptr += size)
		memcpy(ptr, emptyValue, size);
	

	return newData + index * size;
}

void* Array_getAt(Array* array, int index) {
	return array->data + array->size * index;
}

