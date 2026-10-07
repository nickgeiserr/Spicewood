#pragma once
#include <cstdint>
#include <string>
#include "Engine/Unreal.h"
#include <vector>


void InitObjects(uintptr_t moduleBase);
int32_t gObjectsNum();
std::string GetName(uintptr_t nameAddress);
void PrintAllObjects();
uintptr_t GrabObjectAtIndex(int index);
uintptr_t FindObject(const std::string& full_name, bool considerDefaults = false, bool skipTypeClass = true);
std::vector<uintptr_t> FindObjectsByClass(uintptr_t classPtr);
