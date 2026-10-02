#include "pch.h"
#include "Spicewood.h"
#include <format>
#include "Core/Console.h"
#include "Engine/Objects.h"
#include "Hooks.h"

DWORD WINAPI MainThread(LPVOID param) {
	CreateConsole();
	DrawKeybinds({ {"O", "Object count"}, {"L", "Check objects"}, { "K", "FindObject" }, { "J", "FindObjectsByClass" }, { "P", "Unload" }});

	InitObjects((uintptr_t)GetModuleHandle(NULL));
	Print(Startup, "Spicewood loaded");

	int hooksFailed = H_Inititialize((uintptr_t)GetModuleHandle(NULL));
	if (hooksFailed) {
		Print(PrintType::Warning, "MinHook init failed. Hooks will not work.");
	}

	while (true) {
		if ((GetAsyncKeyState('P') & 0x8000) != 0) {
			Print(Info, "Unloading Spicewood");
			H_Shutdown();
			CleanupConsole();
			FreeLibraryAndExitThread((HMODULE)param, 0);
		}

		if ((GetAsyncKeyState('O') & 0x8000) != 0) {
			Print(Info, std::format("GObjects: {}", gObjectsNum()));
		}

		if ((GetAsyncKeyState('L') & 0x8000) != 0) {
			PrintAllObjects();
		}

		if ((GetAsyncKeyState('K') & 0x8000) != 0) {
			Print(PrintType::Debug, std::format("{:#x}",FindObject("Class /Script/SpicewoodGAS.ATR_RangedAttack")));
		}

		if ((GetAsyncKeyState('J') & 0x8000) != 0) {
			uintptr_t classPtr = FindObject("Class /Script/SpicewoodGAS.ATR_RangedAttack");
			if (!classPtr) {
				Print(PrintType::Error, "Failed to find the class pointer.");
			}
			else {
				std::vector<uintptr_t> objects = FindObjectsByClass(classPtr);

				Print(PrintType::Debug, std::to_string(objects.size()));

				for (uintptr_t object : objects) {
					Print(PrintType::Debug, std::format("{:#x}", object) + " | " + GetName(object + 0x18));
				}
			}
		}

		Sleep(150);
	}

	return 0;
}