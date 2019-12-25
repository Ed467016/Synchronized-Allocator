#include <Windows.h>
#include <algorithm>
#include <iostream>
#include "eHeap.h"

#define __MEMORY_SIZE 1024
#define __THREAD_COUNT 2

DWORD WINAPI thread_function(LPVOID lp) {
	auto heap = *(eHeap*)lp;
	while (true)
	{
		auto size = (DWORD)rand() % 32;
		auto iP = heap.newObject(size);
		auto time = (DWORD)rand() % 100;
		Sleep(time);
		heap.deleteObject(iP, size);
	}

	return (DWORD)0;
}

int main() {
	srand((time_t)NULL);
	DWORD lp;
	HANDLE mHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, __MEMORY_SIZE, __MEMORY_SIZE);
	HANDLE mHeapMutex = CreateMutex(NULL, FALSE, NULL);
	eHeap heap(mHeap, mHeapMutex, std::cout, __MEMORY_SIZE);

	HANDLE tHandles[__THREAD_COUNT];
	for (size_t i = 0; i < __THREAD_COUNT; i++) {
		tHandles[i] = CreateThread(NULL, 512, thread_function, &heap, 0, &lp);
	}

	WaitForMultipleObjects(__THREAD_COUNT, tHandles, TRUE, 5000);

	for (size_t i = 0; i < __THREAD_COUNT; i++) {
		TerminateThread(tHandles[i], 0);
		CloseHandle(tHandles[i]);
	}

	return 0;
}