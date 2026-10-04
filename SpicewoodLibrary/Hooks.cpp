#include "pch.h"
#include "Hooks.h"
#include <unordered_set>
#include <string>
#include <sstream>

static ProcessEventFn oProcessEvent = nullptr;
static std::unordered_set<UFunction*> calledFunctions = {};
static uintptr_t* allocatedShadowTable = nullptr;

static int functionCaptureCount = 0;
constexpr int MaxCaptures = 20;

int H_Inititialize(uintptr_t targetInstance) {
	if (!targetInstance) {
		Print(PrintType::Error, "Target object instance is null for shadow hook.");
		return 1;
	}

	uintptr_t** vmtArrayPointer = (uintptr_t**)targetInstance;
	uintptr_t* originalVMT = *vmtArrayPointer;

	constexpr int TableSize = 150;
	constexpr int ProcessEventIdx = 76;

	allocatedShadowTable = new uintptr_t[TableSize];
	memcpy(allocatedShadowTable, originalVMT, TableSize * sizeof(uintptr_t));

	oProcessEvent = (ProcessEventFn)originalVMT[ProcessEventIdx];
	allocatedShadowTable[ProcessEventIdx] = (uintptr_t)&HandleProcessEvent;

	DWORD oldProtect;
	if (!VirtualProtect(vmtArrayPointer, sizeof(uintptr_t), PAGE_READWRITE, &oldProtect)) {
		Print(PrintType::Error, "VirtualProtect failed on table pointer.");
		return 1;
	}

	*vmtArrayPointer = allocatedShadowTable;

	VirtualProtect(vmtArrayPointer, sizeof(uintptr_t), oldProtect, &oldProtect);

	functionCaptureCount = 0;
	Print(PrintType::Info, "ProcessEvent Shadow Hook initialized. Capturing first 20 calls...");
	return 0;
}

void HandleProcessEvent(UObject* object, UFunction* function, void* params) {
	if (functionCaptureCount < MaxCaptures) {
		functionCaptureCount++;

		std::stringstream ss;
		ss << "Intercepted Function Call [" << functionCaptureCount << "/" << MaxCaptures << "] | Function address: 0x" << std::hex << (uintptr_t)function;
		Print(PrintType::Debug, ss.str());
	}

	oProcessEvent(object, function, params);
	return;
}

int H_Shutdown() {
	if (allocatedShadowTable != nullptr) {
		delete[] allocatedShadowTable;
		allocatedShadowTable = nullptr;
		Print(PrintType::Info, "Hooks uninitialized.");
	}
	return 1;
}
