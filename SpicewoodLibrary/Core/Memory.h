#pragma once
#include <cstdint>

template<typename T>
T Read(uintptr_t pointer, uintptr_t offset) {
	return *(T*)((pointer) + offset);
};