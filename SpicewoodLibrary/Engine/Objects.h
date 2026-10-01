#pragma once
#include <cstdint>
#include <string>
#include "Engine/Unreal.h"
#include <vector>


uintptr_t TObjectAddress(uintptr_t base_address);
int32_t gObjectsNum(uintptr_t tObjectArray);
std::string GetName(AppendString append, FString* string, uintptr_t nameAddress);
void PrintAllObjects(uintptr_t tObjectAddress);
uintptr_t FindObject(const std::string& full_name);
std::vector<uintptr_t> FindObjectsByClass(uintptr_t classPtr);
