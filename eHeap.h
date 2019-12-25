#pragma once
#include <Windows.h>
#include <ostream>
#include <map>

struct range {
	size_t from;
	size_t to;

	BOOL contains(size_t);
};

struct eHeap {
	HANDLE mHeap;
	HANDLE mHeapMutex;
	size_t mSize;
	std::map<uintptr_t, range> mUsedSpace;
	std::ostream* mOs;

	eHeap(HANDLE, HANDLE, std::ostream&, size_t);

	void* newObject(size_t);
	BOOL deleteObject(void*, size_t);
	BOOL isFree(size_t);
};
