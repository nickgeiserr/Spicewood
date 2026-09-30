#include "pch.h"
#include "Spicewood.h"
#include <cstdio>
#include <iostream>
#include "Offsets.h"
#include "Memory.h"

void CreateConsole() {
	if (AllocConsole()) {
		FILE* fDummy;

		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
	}

	std::cout << "--- Spicewood Console ---" << std::endl;
}

void Cleanup() {
	FreeConsole();
}

void PrintNumGObjects(uintptr_t baseAddress) {
	uintptr_t TUObjectArray = baseAddress + TOBJECT_ARRAY;
	int32_t numElements = Read<int32_t>((void*)TUObjectArray, 0x14);

	std::cout << "Number of GObjects : " << numElements << std::endl;
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
			PrintNumGObjects(processBaseAddress);
		}

		Sleep(50);
	}

	return 0;
}