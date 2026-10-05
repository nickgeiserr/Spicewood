#include "pch.h"
#include "Hooks.h"
#include <unordered_set>
#include <mutex>
#include <string>
#include <format>
#include <future>

static ProcessEventFn oProcessEvent = nullptr;
static std::unordered_set<UFunction*> calledFunctions = {};
static std::mutex calledFunctionsMutex;

int H_CreateHooks(uintptr_t base) {
	LPVOID* processEvent = (LPVOID*)(base + PROCESS_EVENT);

	MH_STATUS hookStatus = MH_CreateHook(processEvent, &HandleProcessEvent, reinterpret_cast<LPVOID*>(&oProcessEvent));


	if (hookStatus != MH_OK) {
		Print(PrintType::Error, "Failed to hook ProcessEvent");
		return 1;
	}

	Print(PrintType::Info, "ProcessEvent Hooked.");

	return MH_OK;
}

int H_Inititialize(uintptr_t base) {
	if (MH_Initialize() != MH_OK) {
		Print(PrintType::Error, "Failed to initialize MinHook");
		return 1;
	}

	H_CreateHooks(base);

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		Print(PrintType::Error, "Failed to enable hooks.");
		return 1;
	}
	Print(PrintType::Info, "Hooks Enabled.");

	static std::future<void> background_task;
	static std::future<void> nested_background_task;

	background_task = std::async(std::launch::async, [base]() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(9));

			MH_DisableHook(MH_ALL_HOOKS);
			Print(PrintType::Debug, "Hooks disabled.");

			std::this_thread::sleep_for(std::chrono::seconds(3));

			MH_EnableHook(MH_ALL_HOOKS);
			Print(PrintType::Debug, "Hooks re-enabled.");
		}
		});

	return MH_OK;
}

void HandleProcessEvent(UObject* object, UFunction* function, void* params) {
	static thread_local bool bIsInsideHook = false;
	if (bIsInsideHook) {
		oProcessEvent(object, function, params);
		return;
	}

	if (!object || !function) {
		oProcessEvent(object, function, params);
		return;
	}

	bIsInsideHook = true;

	bool alreadyCalled = false;
	{
		std::lock_guard<std::mutex> lock(calledFunctionsMutex);
		if (calledFunctions.contains(function)) {
			alreadyCalled = true;
		}
		else {
			calledFunctions.insert(function);
		}
	}

	if (alreadyCalled) {
		bIsInsideHook = false;
		oProcessEvent(object, function, params);
		return;
	}

	std::string funcName = GetName((uintptr_t)function + 0x18);

	if (funcName.empty() ||
		funcName == "None" ||
		funcName.find("Mouse") != std::string::npos ||
		funcName.find("Tick") != std::string::npos ||
		funcName.find("Animation") != std::string::npos ||
		funcName.find("EvaluateGraph") != std::string::npos ||
		funcName.find("Receive") != std::string::npos)
	{
		bIsInsideHook = false;
		oProcessEvent(object, function, params);
		return;
	}

	Print(PrintType::Debug, "New Function Called: " + funcName);
	if (funcName == "OnProjectileLaunch") {
		Print(PrintType::Info, "Projectile shot!");
	}
	bIsInsideHook = false;
	oProcessEvent(object, function, params);
	return;
}

int H_Shutdown() {
	Print(PrintType::Warning, ">>> H_Shutdown() CALLED <<<");
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	return 1;
}
