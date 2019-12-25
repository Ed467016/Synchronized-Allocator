#include <Windows.h>
#include <ostream>
#include <map>
#include <algorithm>
#include <assert.h>
#include "eHeap.h"

BOOL range::contains(size_t v) {
	return from <= v && v <= to;
}

eHeap::eHeap(
	HANDLE heap,
	HANDLE heapMutex,
	std::ostream& os,
	size_t size)
	: mHeap{ heap }, mHeapMutex{ heapMutex }, mOs{ &os }, mSize{ size }
{
}

void* eHeap::newObject(size_t srcSize) {
	WaitForSingleObject(mHeapMutex, INFINITE);
	int8_t insertingIndex = -1;
	size_t musSize = mUsedSpace.size();
	if (musSize == 0 || mUsedSpace.at(0).from > srcSize) {
		assert(srcSize <= mSize);
		insertingIndex = 0;
	}
	else if (mUsedSpace.at(musSize - 1).to + srcSize <= mSize) {
		insertingIndex = mUsedSpace.at(musSize - 1).to + 1;
	}
	else {
		for (size_t i = 0; i < musSize - 1; i++) {
			if ((mUsedSpace.at(i + 1).from - mUsedSpace.at(i).to - 1) >= srcSize) {
				insertingIndex = mUsedSpace.at(i).to + 1;
				break;
			}
		}
	}
	assert(insertingIndex >= 0);
	auto res = HeapAlloc(mHeap, HEAP_ZERO_MEMORY, srcSize);
	uintptr_t ref = (uintptr_t)res;
	range rn{ insertingIndex, insertingIndex + srcSize };
	*mOs << "Created object at " << ref << " with size " << srcSize << "\n";
	auto it = mUsedSpace.begin();
	if (musSize == 0) {
		mUsedSpace.insert(it, std::pair<uintptr_t, range>(ref, rn));
	}
	else {
		for (size_t i = 0; i < musSize; i++, it++) {
			if (mUsedSpace.at(i).from > rn.from) {
				mUsedSpace.insert(it, std::pair<uintptr_t, range>(ref, rn));
			}
		}
	}

	ReleaseMutex(mHeapMutex);
	return res;
}

BOOL eHeap::deleteObject(void* src, size_t srcSize) {
	WaitForSingleObject(mHeapMutex, INFINITE);
	*mOs << "Deleted an object at " << (const uintptr_t)src << "\n";
	auto it = mUsedSpace.find((const uintptr_t)src);
	assert(it != mUsedSpace.cend());
	mUsedSpace.erase(it);
	auto res = HeapFree(mHeap, HEAP_ZERO_MEMORY, src);
	ReleaseMutex(mHeapMutex);
	return res;
}

BOOL eHeap::isFree(size_t byte) {
	for (size_t i = 0; i < mUsedSpace.size(); i++) {
		auto el = mUsedSpace.at(i);
		if (el.from > byte) {
			return true;
		}
		if (el.contains(byte)) {
			return false;
		}
	}
}
