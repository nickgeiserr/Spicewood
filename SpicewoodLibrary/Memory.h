#pragma once
#include <cstdint>

template<typename T>
T Read(void* pointer, uintptr_t offset) {
	return *(T*)(((char*)pointer) + offset);
};