#pragma once

#include "external/minhook/include/MinHook.h"
#include "Core/Console.h"
#include "Engine/Unreal.h"
#include "Engine/Offsets.h"
#include "Engine/Objects.h"

int H_Inititialize(uintptr_t base);
 
int H_Shutdown();
void HandleProcessEvent(UObject* object, UFunction* function, void* params);