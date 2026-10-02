#include "pch.h"
#include "Hooks.h"
#include <unordered_set>

static ProcessEventFn oProcessEvent = nullptr;
static std::unordered_set<UFunction*> calledFunctions = {};

int H_Inititialize(uintptr_t base) {
	if (MH_Initialize() != MH_OK) {
		Print(PrintType::Error, "Failed to initialize MinHook");
		return 1;
	}

	LPVOID* processEvent = (LPVOID*)(base + PROCESS_EVENT);

	MH_STATUS hookStatus = MH_CreateHook(processEvent, &HandleProcessEvent, reinterpret_cast<LPVOID*>(&oProcessEvent));

	 if (hookStatus != MH_OK) {
	 	Print(PrintType::Error, "Failed to hook ProcessEvent");
	  	return 1;
	 }

	Print(PrintType::Info, "ProcessEvent Hooked.");

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		Print(PrintType::Error, "Failed to enable hooks.");
		return 1;
	}
	Print(PrintType::Info, "Hooks Enabled.");
	

	return MH_OK;
}

void HandleProcessEvent(UObject* object, UFunction* function, void* params) {
	if (calledFunctions.contains(function)) {
		oProcessEvent(object, function, params);
		return;
	}

	Print(PrintType::Debug, "New Function Called: " + GetName((uintptr_t)function + 0x18));
	calledFunctions.insert(function);
	oProcessEvent(object, function, params);
	return;
}

int H_Shutdown() {
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	return 1;
}