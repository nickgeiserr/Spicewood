#pragma once
#include <cstdint>

const uint32_t PAGE_MAX = 65536;

struct FString {
	wchar_t* data;
	INT32 num;
	INT32 max;
};
static_assert(sizeof(FString) == 0x10);

// forward dec
class UObject;
class UFunction;
class UClass;


using AppendString = void(*)(const void*, FString&);
using ProcessEventFn = void(__fastcall *)(UObject* object, UFunction* function, void* params);
