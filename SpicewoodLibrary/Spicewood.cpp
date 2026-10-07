#include "pch.h"
#include "Spicewood.h"
#include <format>
#include "Core/Console.h"
#include "Engine/Objects.h"
#include "Hooks.h"


DWORD WINAPI MainThread(LPVOID param) {
	CreateConsole();
	DrawKeybinds({ {"F9", "Object count"}, {"F8", "Check objects"}, {"F7", "FindObject"}, {"F6", "Equip Item"}, {"F10", "Unload"} });
	SetDebugMode(true);

	uintptr_t moduleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
	InitObjects(moduleBase);
	Hooks::Initialize(moduleBase);
	Print(Startup, "Spicewood loaded");

	while (true) {
		if ((GetAsyncKeyState(VK_F10) & 0x8000) != 0) {
			Print(Info, "Unloading Spicewood");
			Hooks::Shutdown();
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
			Print(PrintType::Debug, std::format("{:#x}", FindObject("Class /Script/SpicewoodGAS.ATR_RangedAttack", false)));
		}

		if ((GetAsyncKeyState(VK_F6) & 0x8000) != 0) {
			struct FSWSessionUID {
				uint64_t UID;
			};

			struct UInventoryManagerComponent_EquipItemInAppropriateSlot_Params {
				FSWSessionUID UID;
				bool ReturnValue;
			};

			uintptr_t object = GrabObjectAtIndex(103060);
			uintptr_t function = FindObject("Function /Script/InventorySystem.InventoryManagerComponent.EquipItemInAppropriateSlot", false, true);
			if (!function || !object) {
				Print(PrintType::Debug, "Failed to resolve object or function pointer");
			}

			UInventoryManagerComponent_EquipItemInAppropriateSlot_Params params{ {7316}, false };
			Hooks::CallProcessEvent(
				reinterpret_cast<UObject*>(object),
				reinterpret_cast<UFunction*>(function),
				&params
			);

			Print(PrintType::Debug, "Called process event");
		}


		Sleep(150);
	}

	return 0;
}
