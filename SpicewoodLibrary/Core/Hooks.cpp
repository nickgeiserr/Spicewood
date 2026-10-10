#include "pch.h"
#include "Hooks.h"
#include "Console.h"
#include "Memory.h"
#include <algorithm>
#include <atomic>
#include <mutex>
#include <new>
#include <vector>

namespace Hooks {
    namespace {
        constexpr size_t ProcessEventVTableIndex = 0x4C;
        constexpr size_t MaximumVTableSize = 1024;
        constexpr uintptr_t ClassOffset = 0x10;
        constexpr uintptr_t ObjectFlagsOffset = 0x08;
        constexpr uintptr_t SuperStructOffset = 0x40;
        constexpr uint32_t ProcessEventFlag = 0x400;

        enum class HookKind {
            None,
            PlayerCharacter
        };

        struct HookRecord {
            UObject* object;
            void** originalVTable;
            void** shadowAllocation;
            void** shadowVTable;
            ProcessEventFn originalProcessEvent;
            HookKind kind;
        };

        static ProcessEventFn g_ProcessEventRaw = nullptr;
        static UFunction* g_ReceiveTickFunction = nullptr;
        static UClass* g_PlayerCharacterClass = nullptr;
        static std::vector<HookRecord> g_Hooks;
        static std::mutex g_HookMutex;
        static std::mutex g_TaskMutex;
        static std::vector<std::function<void()>> g_TaskQueue;
        static std::atomic_bool g_TickReported = false;
        static std::atomic_uint32_t g_ActiveCallbacks = 0;
        static bool g_IsInitialized = false;
        static bool g_IsHooked = false;
        static ULONGLONG g_LastScan = 0;

        bool IsReadableAddress(uintptr_t address) {
            MEMORY_BASIC_INFORMATION memoryInfo{};
            if (!VirtualQuery(reinterpret_cast<void*>(address), &memoryInfo, sizeof(memoryInfo)) ||
                memoryInfo.State != MEM_COMMIT ||
                (memoryInfo.Protect & (PAGE_NOACCESS | PAGE_GUARD)) != 0) {
                return false;
            }

            return true;
        }

        bool IsExecutableAddress(uintptr_t address) {
            MEMORY_BASIC_INFORMATION memoryInfo{};
            if (!VirtualQuery(reinterpret_cast<void*>(address), &memoryInfo, sizeof(memoryInfo)) ||
                memoryInfo.State != MEM_COMMIT) {
                return false;
            }

            DWORD protection = memoryInfo.Protect & 0xFF;
            return protection == PAGE_EXECUTE ||
                protection == PAGE_EXECUTE_READ ||
                protection == PAGE_EXECUTE_READWRITE ||
                protection == PAGE_EXECUTE_WRITECOPY;
        }

        size_t GetVTableSize(void** vTable) {
            if (!vTable)
                return 0;

            size_t size = 0;
            while (size < MaximumVTableSize) {
                uintptr_t entryAddress = reinterpret_cast<uintptr_t>(vTable + size);
                if (!IsReadableAddress(entryAddress) ||
                    !IsReadableAddress(entryAddress + sizeof(void*) - 1)) {
                    break;
                }

                uintptr_t functionAddress = reinterpret_cast<uintptr_t>(vTable[size]);
                if (!IsExecutableAddress(functionAddress))
                    break;

                size++;
            }

            return size;
        }

        HookRecord* FindHook(UObject* object) {
            auto it = std::find_if(g_Hooks.begin(), g_Hooks.end(), [object](const HookRecord& hook) {
                return hook.object == object;
            });
            return it == g_Hooks.end() ? nullptr : &*it;
        }

        HookKind GetHookKind(uintptr_t object) {
            uintptr_t classAddress = Read<uintptr_t>(object, ClassOffset);

            for (size_t i = 0; classAddress && i < 64; i++) {
                if (classAddress == reinterpret_cast<uintptr_t>(g_PlayerCharacterClass))
                    return HookKind::PlayerCharacter;

                classAddress = Read<uintptr_t>(classAddress, SuperStructOffset);
            }

            return HookKind::None;
        }

        void __fastcall hkFunctionProcessEvent(UObject* object, UFunction* function, void* params) {
            g_ActiveCallbacks.fetch_add(1);
            ProcessEventFn original = nullptr;
            HookKind hookKind = HookKind::None;
            bool isReceiveTick = false;

            {
                std::lock_guard<std::mutex> lock(g_HookMutex);
                HookRecord* hook = FindHook(object);
                if (hook) {
                    original = hook->originalProcessEvent;
                    hookKind = hook->kind;
                }
                isReceiveTick = function == g_ReceiveTickFunction;
            }

            if (!isReceiveTick && function)
                isReceiveTick = GetName(reinterpret_cast<uintptr_t>(function) + 0x18) == "ReceiveTick";

            if (isReceiveTick && hookKind == HookKind::PlayerCharacter) {
                std::vector<std::function<void()>> localTasks;
                {
                    std::lock_guard<std::mutex> lock(g_TaskMutex);
                    localTasks.swap(g_TaskQueue);
                }

                for (auto& task : localTasks) {
                    if (task)
                        task();
                }

                if (!g_TickReported.exchange(true))
                    Print(PrintType::Debug, "AActor.ReceiveTick intercepted");
            }

            if (original)
                original(object, function, params);

            g_ActiveCallbacks.fetch_sub(1);
        }

        bool InstallHook(uintptr_t objectAddress, HookKind hookKind) {
            auto* object = reinterpret_cast<UObject*>(objectAddress);
            std::lock_guard<std::mutex> lock(g_HookMutex);

            if (FindHook(object))
                return true;

            void** originalVTable = *reinterpret_cast<void***>(object);
            size_t vTableSize = GetVTableSize(originalVTable);
            if (vTableSize <= ProcessEventVTableIndex ||
                !IsReadableAddress(reinterpret_cast<uintptr_t>(originalVTable) - sizeof(void*))) {
                Print(PrintType::Error, "Could not find ProcessEvent in target vtable");
                return false;
            }

            auto** shadowAllocation = new (std::nothrow) void*[vTableSize + 1];
            if (!shadowAllocation)
                return false;

            shadowAllocation[0] = originalVTable[-1];
            auto** shadowVTable = shadowAllocation + 1;
            memcpy(shadowVTable, originalVTable, vTableSize * sizeof(void*));
            auto originalProcessEvent = reinterpret_cast<ProcessEventFn>(originalVTable[ProcessEventVTableIndex]);
            shadowVTable[ProcessEventVTableIndex] = reinterpret_cast<void*>(&hkFunctionProcessEvent);

            g_Hooks.push_back({ object, originalVTable, shadowAllocation, shadowVTable, originalProcessEvent, hookKind });
            *reinterpret_cast<void***>(object) = shadowVTable;
            g_IsHooked = true;

            return true;
        }

        bool IsLiveObject(uintptr_t objectAddress) {
            int32_t objectCount = gObjectsNum();
            for (int32_t i = 0; i < objectCount; i++) {
                if (TryGrabObjectAtIndex(i) == objectAddress)
                    return true;
            }
            return false;
        }
    }

    bool Initialize(uintptr_t baseAddress) {
        if (g_IsInitialized)
            return true;

        g_ProcessEventRaw = reinterpret_cast<ProcessEventFn>(baseAddress + PROCESS_EVENT);
        g_ReceiveTickFunction = reinterpret_cast<UFunction*>(FindObject(
            "Function /Script/Engine.Actor.ReceiveTick",
            false,
            true
        ));
        g_PlayerCharacterClass = reinterpret_cast<UClass*>(FindObject(
            "Class /Script/Dungeons.PlayerCharacter",
            false,
            false
        ));
        if (!g_ProcessEventRaw ||
            !g_ReceiveTickFunction ||
            !g_PlayerCharacterClass) {
            Print(PrintType::Error, "Failed to resolve ProcessEvent, ReceiveTick, or PlayerCharacter");
            g_ProcessEventRaw = nullptr;
            g_ReceiveTickFunction = nullptr;
            g_PlayerCharacterClass = nullptr;
            return false;
        }

        g_IsInitialized = true;
        g_LastScan = 0;
        return true;
    }

    void Update() {
        if (!g_IsInitialized)
            return;

        ULONGLONG now = GetTickCount64();
        if (now - g_LastScan < 1000)
            return;
        g_LastScan = now;

        int32_t objectCount = gObjectsNum();
        for (int32_t i = 0; i < objectCount; i++) {
            uintptr_t object = TryGrabObjectAtIndex(i);
            if (!object)
                continue;

            if ((Read<uint32_t>(object, ObjectFlagsOffset) & 0x10) != 0)
                continue;

            HookKind hookKind = GetHookKind(object);
            if (hookKind == HookKind::None)
                continue;

            InstallHook(object, hookKind);
        }
    }

    void QueueGameThreadTask(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(g_TaskMutex);
        g_TaskQueue.push_back(std::move(task));
    }

    void CallProcessEvent(UObject* object, UFunction* function, void* params) {
        if (g_ProcessEventRaw && object && function) {
            auto functionFlags = reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(function) + 0xB0);
            uint32_t oldFlags = *functionFlags;
            *functionFlags |= ProcessEventFlag;
            g_ProcessEventRaw(object, function, params);
            *functionFlags = oldFlags;
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

        {
            std::lock_guard<std::mutex> lock(g_HookMutex);
            for (auto& hook : g_Hooks) {
                if (IsLiveObject(reinterpret_cast<uintptr_t>(hook.object)) &&
                    *reinterpret_cast<void***>(hook.object) == hook.shadowVTable) {
                    *reinterpret_cast<void***>(hook.object) = hook.originalVTable;
                }
            }
        }

        Sleep(100);
        while (g_ActiveCallbacks.load() != 0)
            Sleep(1);

        std::vector<HookRecord> hooks;
        {
            std::lock_guard<std::mutex> lock(g_HookMutex);
            hooks.swap(g_Hooks);
        }

        for (auto& hook : hooks)
            delete[] hook.shadowAllocation;

        g_ProcessEventRaw = nullptr;
        g_ReceiveTickFunction = nullptr;
        g_PlayerCharacterClass = nullptr;
        g_IsInitialized = false;
        g_IsHooked = false;
        g_TickReported = false;
        g_LastScan = 0;
    }
}
