#pragma once

#include "external/minhook/include/MinHook.h"
#include "Core/Console.h"
#include "Engine/Unreal.h"
#include "Engine/Offsets.h"
#include "Engine/Objects.h"
#include "ProcessEvent.h"

#include <cstdint>
#include <functional>

class UObject;
class UFunction;

using ProcessEventFn = void(__fastcall*)(UObject* object, UFunction* function, void* params);

namespace Hooks {
    bool Initialize(uintptr_t baseAddress);
    void Shutdown();

    void CallProcessEvent(UObject* object, UFunction* function, void* params);
    void QueueGameThreadTask(std::function<void()> task);

    bool IsHooked();
}
