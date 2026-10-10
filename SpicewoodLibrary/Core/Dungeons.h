#pragma once
#include <cstdint>

struct FScriptArray {
	void* Data;
	int32_t Num;
	int32_t Max;
};

struct FInventoryEntry {
	uint8_t ItemData[0xC0];
	int32_t StackCount;
	uint8_t EquippedSlot[0x8];
	uint8_t Pad_CC[0x4];
	FScriptArray MetaData;
	uint8_t Pad_E0[0x8];
};

struct GetInventoryItemsParams {
	FScriptArray ReturnValue;
};

static_assert(sizeof(FScriptArray) == 0x10);
static_assert(sizeof(FInventoryEntry) == 0xE8);
static_assert(sizeof(GetInventoryItemsParams) == 0x10);