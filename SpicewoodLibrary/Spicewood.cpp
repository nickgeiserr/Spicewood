#include "pch.h"
#include "Spicewood.h"
#include <format>
#include "Core/Console.h"
#include "Engine/Objects.h"
#include "Hooks.h"
#include <TlHelp32.h>
#include <fstream>
#include <Aegis.h>

void LogAllProcessThreadsToFile() {
	DWORD currentPID = GetCurrentProcessId();

	std::ofstream logFile("C:\\Users\\Public\\thread_log.txt", std::ios::out | std::ios::app);
	if (!logFile.is_open()) {
		return;
	}

	HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnapshot == INVALID_HANDLE_VALUE) {
		logFile << "[!] Failed to create thread snapshot.\n";
		logFile.close();
		return;
	}

	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	logFile << "=== Enumerating Active Process Threads ===\n";

	if (Thread32First(hThreadSnapshot, &te32)) {
		do {
			if (te32.th32OwnerProcessID == currentPID) {
				std::string logLine = "Thread ID: " + std::to_string(te32.th32ThreadID);

				HANDLE hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, te32.th32ThreadID);
				if (hThread != NULL) {
					PWSTR threadDescription = nullptr;
					HRESULT hr = GetThreadDescription(hThread, &threadDescription);
					if (SUCCEEDED(hr) && threadDescription != nullptr && *threadDescription != L'\0') {
						std::wstring wDescription(threadDescription);
						std::string sDescription(wDescription.begin(), wDescription.end());
						logLine += " | Name: [" + sDescription + "]";
						LocalFree(threadDescription);
					}
					else {
						logLine += " | Name: [Unnamed Thread / Pool Worker]";
					}
					CloseHandle(hThread);
				}
				else {
					logLine += " | Name: [Access Denied - Security Lock]";
				}

				logFile << logLine << "\n";
			}
		} while (Thread32Next(hThreadSnapshot, &te32));
	}

	logFile << "==========================================\n\n";
	logFile.close();
	CloseHandle(hThreadSnapshot);
}

DWORD WINAPI MainThread(LPVOID param) {
	// UnlinkDllFromPEB((HINSTANCE)param);
	CreateConsole();
	DrawKeybinds({ {"F9", "Object count"}, {"F8", "Check objects"}, { "F7", "FindObject" }, { "F10", "Unload" }});

	InitObjects((uintptr_t)GetModuleHandle(NULL));
	Print(Startup, "Spicewood loaded");

	int hooksFailed = H_Inititialize((uintptr_t)GetModuleHandle(NULL));
	if (hooksFailed) {
	 	Print(PrintType::Warning, "MinHook init failed. Hooks will not work.");
	 }

	LogAllProcessThreadsToFile();

	while (true) {
		if ((GetAsyncKeyState(VK_F10) & 0x8000) != 0) {
			Print(Info, "Unloading Spicewood");
			H_Shutdown();
			CleanupConsole();
			FreeLibraryAndExitThread((HMODULE)param, 0);
		}

		if ((GetAsyncKeyState(VK_F9) & 0x8000) != 0) {
			Print(Info, std::format("GObjects: {}", gObjectsNum()));
		}

		if ((GetAsyncKeyState(VK_F8) & 0x8000) != 0) {
			PrintAllObjects();
		}

		if ((GetAsyncKeyState(VK_F7) & 0x8000) != 0) {
			Print(PrintType::Debug, std::format("{:#x}",FindObject("Class /Script/SpicewoodGAS.ATR_RangedAttack")));
		}



		Sleep(150);
	}

	return 0;
}