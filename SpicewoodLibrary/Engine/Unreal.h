#pragma once
#include <cstdint>

const uint32_t PAGE_MAX = 65536;

struct FString {
	wchar_t* data;
	INT32 num;
	INT32 max;
};
static_assert(sizeof(FString) == 0x10);


using AppendString = void(*)(const void*, FString&);
