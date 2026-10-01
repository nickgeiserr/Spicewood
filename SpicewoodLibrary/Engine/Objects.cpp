#include "pch.h"
#include "Engine/Objects.h"
#include <format>
#include "Core/Memory.h"
#include "Core/Console.h"
#include "Core/Strings.h"
#include "Engine/Offsets.h"

uintptr_t TObjectAddress(uintptr_t base_address) {
	return base_address + TOBJECT_ARRAY;
}

int32_t gObjectsNum(uintptr_t tObjectArray) {
	int32_t numElements = Read<int32_t>(tObjectArray, 0x14);
	return numElements;
}

std::string GetName(AppendString append, FString* string, uintptr_t nameAddress) {
	append((void*)nameAddress, *string);

	std::string name = ToUtf8(string->data);
	string->num = 0;
	return name;
}

std::vector<uintptr_t> FindObjectsByClass(uintptr_t classPtr) {
	uintptr_t tObjectAddress = TObjectAddress((uintptr_t)GetModuleHandle(NULL));
	if (!tObjectAddress) {
		Print(Error, "Failed to grab object table address.");
		return {};
	}

	uintptr_t tocPointer = Read<uintptr_t>(tObjectAddress, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return {};
	}

	uint32_t numObjects = gObjectsNum(tObjectAddress);

	std::vector<uintptr_t> pointer_table = {};

	for (uint32_t i = 0; i < numObjects; i++) {
		int chunk = i / PAGE_MAX;
		int index = i % PAGE_MAX;

		uintptr_t chunkPtr = Read<uintptr_t>(tocPointer, chunk * 8);
		if (!chunkPtr) {
			Print(Warning, std::format("Chunk {} is null, skipping object {}", chunk, i));
			continue;
		}

		uintptr_t objectPtr = Read<uintptr_t>(chunkPtr, index * 24);
		if (!objectPtr) {
			continue;
		}

		uintptr_t classAddress = Read<uintptr_t>(objectPtr, 0x10);
	
		if (classPtr == classAddress) {
			pointer_table.push_back(objectPtr);
		}
	}

	return pointer_table;

}

uintptr_t FindObject(const std::string& full_name) {
	uintptr_t tObjectAddress = TObjectAddress((uintptr_t)GetModuleHandle(NULL));
	if (!tObjectAddress) {
		Print(Error, "Failed to grab object table address.");
		return 0;
	}

	uintptr_t tocPointer = Read<uintptr_t>(tObjectAddress, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return 0;
	}

	uintptr_t appendAddress = (uintptr_t)GetModuleHandle(NULL) + 0x012E5160;

	AppendString append = (AppendString)appendAddress;

	uint32_t numObjects = gObjectsNum(tObjectAddress);

	size_t pos = full_name.rfind('.');
	std::string short_name;
	if (pos != std::string::npos) {
		short_name = full_name.substr(pos + 1);
	}
	else {
		pos = full_name.rfind(' ');
		short_name = full_name.substr(pos + 1);
	}

	for (uint32_t i = 0; i < numObjects; i++) {
		int chunk = i / PAGE_MAX;
		int index = i % PAGE_MAX;

		uintptr_t chunkPtr = Read<uintptr_t>(tocPointer, chunk * 8);
		if (!chunkPtr) {
			Print(Warning, std::format("Chunk {} is null, skipping object {}", chunk, i));
			continue;
		}

		uintptr_t objectPtr = Read<uintptr_t>(chunkPtr, index * 24);
		if (!objectPtr) {
			continue;
		}

		uintptr_t initialNameAddress = objectPtr + 0x18;
		uintptr_t classAddress = Read<uintptr_t>(objectPtr, 0x10);

		wchar_t buffer[1024];
		FString result{ buffer, 0 , 1024 };

		std::string constructedFullName = GetName(append, &result, initialNameAddress);

		if (constructedFullName != short_name) {
			continue;
		}

		bool moreOuter = true;
		uintptr_t nextOuterAddress = initialNameAddress + 0x08;
		while (moreOuter) {
			uintptr_t outer = Read<uintptr_t>(nextOuterAddress, 0x00);
			if (outer) {
				uintptr_t nameAddress = outer + 0x18;
				constructedFullName = GetName(append, &result, nameAddress) + "." + constructedFullName;
				nextOuterAddress = nameAddress + 0x08;
				continue;
			}

			moreOuter = false;
			break;
		}

		uintptr_t classNameAddress = classAddress + 0x18;
		std::string class_name = GetName(append, &result, classNameAddress);

		constructedFullName = class_name + " " + constructedFullName;

		if (constructedFullName == full_name) {
			return objectPtr;
		}
	}

	return 0;
}

void PrintAllObjects(uintptr_t tObjectAddress) {

	Print(Info, "Checking GObjects...");
	uint32_t correct = 0;
	uint32_t wrong = 0;
	uint32_t empty = 0;

	uintptr_t tocPointer = Read<uintptr_t>(tObjectAddress, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return;
	}

	uintptr_t appendAddress = (uintptr_t)GetModuleHandle(NULL) + 0x012E5160;

	AppendString append = (AppendString)appendAddress;

	uint32_t numObjects = gObjectsNum(tObjectAddress);
	for (int i = 0; i < 25; i++) {
		int chunk = i / PAGE_MAX;
		int index = i % PAGE_MAX;

		uintptr_t chunkPtr = Read<uintptr_t>(tocPointer, chunk * 8);
		if (!chunkPtr) {
			Print(Warning, std::format("Chunk {} is null, skipping object {}", chunk, i));
			continue;
		}

		uintptr_t objectPtr = Read<uintptr_t>(chunkPtr, index * 24);
		if (!objectPtr) {
			empty += 1;
			continue;
		}

		uintptr_t initialNameAddress = objectPtr + 0x18;
		uintptr_t classAddress = Read<uintptr_t>(objectPtr, 0x10);

		wchar_t buffer[1024];
		FString result{ buffer, 0 , 1024 };

		std::string full_name = GetName(append, &result, initialNameAddress);

		bool moreOuter = true;
		uintptr_t nextOuterAddress = initialNameAddress + 0x08;
		while (moreOuter) {
			uintptr_t outer = Read<uintptr_t>(nextOuterAddress, 0x00);
			if (outer) {
				uintptr_t nameAddress = outer + 0x18;
				full_name = GetName(append, &result, nameAddress) + "." + full_name;
				nextOuterAddress = nameAddress + 0x08;
				continue;
			}

			moreOuter = false;
			break;
		}

		

		uintptr_t classNameAddress = classAddress + 0x18;
		std::string class_name = GetName(append, &result, classNameAddress);

		full_name = class_name + " " + full_name;

		Print(PrintType::Debug, full_name);

		int32_t trueIndex = Read<int32_t>(objectPtr, 0x0c);
		if (trueIndex == i) {
			correct += 1;
		}
		else {
			wrong += 1;
		}

	}

	Print(wrong == 0 ? Startup : Warning, std::format("{} correct, {} wrong, {} empty slots", correct, wrong, empty));
}
