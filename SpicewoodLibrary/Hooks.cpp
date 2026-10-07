#include "pch.h"
#include "Hooks.h"
#include "Core/Console.h"
#include <mutex>
#include <vector>

namespace Hooks {
    static ProcessEventFn g_ProcessEventRaw = nullptr;
    static ProcessEventFn g_OriginalFuncProcessEvent = nullptr;

    static void** g_OriginalFuncVTable = nullptr;
    static void** g_ReplacedFuncVTable = nullptr;
    static void* g_HookedFuncInstance = nullptr;

    static std::mutex g_TaskMutex;
    static std::vector<std::function<void()>> g_TaskQueue;
    static bool g_IsHooked = false;

    static void __fastcall hkFunctionProcessEvent(UObject* object, UFunction* function, void* params) {
        std::vector<std::function<void()>> localTasks;
        {
            std::lock_guard<std::mutex> lock(g_TaskMutex);
            if (!g_TaskQueue.empty()) {
                localTasks.swap(g_TaskQueue);
            }
        }

        for (auto& task : localTasks) {
            if (task) {
                task();
            }
        }

        if (g_OriginalFuncProcessEvent) {
            g_OriginalFuncProcessEvent(object, function, params);
        }
    }

    bool Initialize(uintptr_t baseAddress) {
        uintptr_t peAddress = baseAddress + PROCESS_EVENT;
        g_ProcessEventRaw = reinterpret_cast<ProcessEventFn>(peAddress);

        if (!g_ProcessEventRaw) {
            Print(PrintType::Error, "Failed to resolve raw ProcessEvent address.");
            return false;
        }

        uintptr_t targetFunction = FindObject("Function /Script/Engine.HUD.ReceiveDrawHUD", false, true);
        if (!targetFunction) {
            targetFunction = FindObject("Function /Script/Engine.Actor.ReceiveTick", false, true);
        }

        if (!targetFunction) {
            Print(PrintType::Error, "Failed to resolve a safe UFunction target for VMT migration.");
            return false;
        }

        g_HookedFuncInstance = reinterpret_cast<void*>(targetFunction);

        g_OriginalFuncVTable = *reinterpret_cast<void***>(g_HookedFuncInstance);

        size_t vtableSize = 0;
        while (g_OriginalFuncVTable[vtableSize] != nullptr && vtableSize < 150) {
            vtableSize++;
        }

        g_ReplacedFuncVTable = new void* [vtableSize];
        memcpy(g_ReplacedFuncVTable, g_OriginalFuncVTable, vtableSize * sizeof(void*));

        size_t processEventVmtIndex = 76;
        g_OriginalFuncProcessEvent = reinterpret_cast<ProcessEventFn>(g_OriginalFuncVTable[processEventVmtIndex]);
        g_ReplacedFuncVTable[processEventVmtIndex] = reinterpret_cast<void*>(&hkFunctionProcessEvent);

        *reinterpret_cast<void***>(g_HookedFuncInstance) = g_ReplacedFuncVTable;

        g_IsHooked = true;
        Print(PrintType::Debug, "Target UFunction VMT redirected. Inline integrity preserved.");
        return true;
    }

    void QueueGameThreadTask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(g_TaskMutex);
        g_TaskQueue.push_back(std::move(task));
    }

    void CallProcessEvent(UObject* object, UFunction* function, void* params) {
        if (g_ProcessEventRaw && object && function) {
            g_ProcessEventRaw(object, function, params);
        }
    }

    bool IsHooked() {
        return g_IsHooked;
    }

    void Shutdown() {
        {
            std::lock_guard<std::mutex> lock(g_TaskMutex);
            g_TaskQueue.clear();
        }

        if (g_HookedFuncInstance && g_OriginalFuncVTable) {
            *reinterpret_cast<void***>(g_HookedFuncInstance) = g_OriginalFuncVTable;
        }

        if (g_ReplacedFuncVTable) {
            delete[] g_ReplacedFuncVTable;
            g_ReplacedFuncVTable = nullptr;
        }

        g_HookedFuncInstance = nullptr;
        g_OriginalFuncVTable = nullptr;
        g_OriginalFuncProcessEvent = nullptr;
        g_ProcessEventRaw = nullptr;
        g_IsHooked = false;

        Print(PrintType::Info, "Hooks cleanly uninitialized.");
    }
}
