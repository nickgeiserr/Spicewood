#pragma once
#include <Windows.h>

DWORD WINAPI MainThread(LPVOID param);
void CreateConsole();
void Cleanup();
