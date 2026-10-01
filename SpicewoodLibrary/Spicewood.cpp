#include "pch.h"
#include "Spicewood.h"
#include <cstdio>
#include <iostream>
#include "Offsets.h"
#include "Memory.h"
#include "Unreal.h"

void CreateConsole() {
	if (AllocConsole()) {
		FILE* fDummy;

		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		SetConsoleTitleA("Spicewood - Library");
		
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleMode(consoleHandle, ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}

	std::cout << "--- Spicewood Console ---" << std::endl;
}

void Cleanup() {
	FreeConsole();
}

uintptr_t TObjectAddress(uintptr_t base_address) {
	return base_address + TOBJECT_ARRAY;
}

int32_t gobjectsNum(uintptr_t tObjectArray) {
	int32_t numElements = Read<int32_t>((void*)tObjectArray, 0x14);
	return numElements;
}

void PrintAllObjectsToFile(uintptr_t tObjectAddress) {

	std::cout << "Printing." << std::endl;
	uint32_t correct = 0;
	uint32_t wrong = 0;

	void* tocPointer = Read<void*>((void*)tObjectAddress, 0x00);
	if (tocPointer == nullptr) {
		std::cout << "Failed to grab table of contents. " << std::endl;
		return;
	}
	uint32_t numObjects = gobjectsNum(tObjectAddress);
	for (int i = 0; i < numObjects; i++) {
		int chunk = i / PAGE_MAX;
		int index = i % PAGE_MAX;

		void* chunkPtr = Read<void*>((void*)tocPointer, chunk * 8);
		if (chunkPtr == nullptr) {
			std::cout << "Failed to grab chunk pointer. Skipping";
			continue;
		}

		void* objectPtr = Read<void*>(chunkPtr, index * 24);
		if (objectPtr == nullptr) {
			std::cout << "Failed to grab object pointer. Skipping";
			continue;
		}

		int32_t trueIndex = Read<int32_t>(objectPtr, 0x0c);
		if (trueIndex == i) {
			correct += 1;
		}
		else {
			wrong += 1;
		}

	}

	std::cout << std::endl << "There was " << correct << " correct numbers and " << wrong << " wrong numbers. Nice? maybe.";
}

DWORD WINAPI MainThread(LPVOID param) {
	CreateConsole();
	uintptr_t processBaseAddress = (uintptr_t)GetModuleHandle(NULL);

	while (true) {
		if ((GetAsyncKeyState('P') & 0x8000) != 0) {
			std::cout << "Exiting Spicewood" << std::endl;
			Cleanup();
			FreeLibraryAndExitThread((HMODULE)param, 0);
		}

		if ((GetAsyncKeyState('O') & 0x8000) != 0) {
			std::cout << gobjectsNum(TObjectAddress(processBaseAddress));
		}

		if ((GetAsyncKeyState('L') & 0x8000) != 0) {
			PrintAllObjectsToFile(TObjectAddress(processBaseAddress));
		}

		Sleep(150);
	}

	return 0;
}