#include "pch.h"
#include "Spicewood.h"
#include <format>
#include "Core/Console.h"
#include "Engine/Objects.h"
#include "Core/Hooks.h"
#include <ModLoader/ModLoader.h>

DWORD WINAPI MainThread(LPVOID param) {
	CreateConsole();
	SetDebugMode(true);

	Print(PrintType::Startup, "Loading Spicewood");

	uintptr_t moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
	InitObjects(moduleBase);

	bool hooksActive = Hooks::Initialize(moduleBase);
	if (!hooksActive) {
		Print(PrintType::Warning, "Failed to initialize hooks. Hooks won't work. ");
	}

	ModLoader::LoadMods();

	Print(Info, "Spicewood Loaded");

	while (true) {
		Hooks::Update();

		if ((GetAsyncKeyState(VK_F10) & 0x8000) != 0) {
			Print(Info, "Unloading Spicewood");
			Hooks::Shutdown();
			ModLoader::Shutdown();
			CleanupConsole();
			FreeLibraryAndExitThread((HMODULE)param, 0);
		}

		Sleep(150);
	}

	return 0;
}
