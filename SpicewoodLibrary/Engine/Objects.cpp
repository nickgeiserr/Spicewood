#include "pch.h"
#include "Engine/Objects.h"
#include <format>
#include "Core/Memory.h"
#include "Core/Console.h"
#include "Core/Strings.h"
#include "Engine/Offsets.h"
#include <fstream>
#include "../Hooks.h"

static uintptr_t moduleBase = 0;
static uintptr_t objectArray = 0;
static AppendString appendString = nullptr;

void InitObjects(uintptr_t base) {
	moduleBase = base;
	objectArray = base + TOBJECT_ARRAY;
	appendString = (AppendString)(base + APPEND_STRING);
}

int32_t gObjectsNum() {
	if (!objectArray)
		return 0;

	int32_t numElements = Read<int32_t>(objectArray, 0x14);
	return numElements;
}

std::string GetName(uintptr_t nameAddress) {
	if (!appendString)
		return "";

	wchar_t buffer[1024];
	FString string{ buffer, 0, 1024 };
	appendString((void*)nameAddress, string);

	return ToUtf8(string.data);
}

std::vector<uintptr_t> FindObjectsByClass(uintptr_t classPtr) {
	if (!objectArray) {
		Print(Error, "Failed to grab object table address.");
		return {};
	}

	uintptr_t tocPointer = Read<uintptr_t>(objectArray, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return {};
	}

	uint32_t numObjects = gObjectsNum();

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

uintptr_t FindObject(const std::string& full_name, bool considerDefaults, bool skipTypeClass) {
	if (!objectArray) {
		Print(Error, "Failed to grab object table address.");
		return 0;
	}

	uintptr_t tocPointer = Read<uintptr_t>(objectArray, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return 0;
	}

	uint32_t numObjects = gObjectsNum();

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
			continue;
		}

		uintptr_t objectPtr = Read<uintptr_t>(chunkPtr, index * 24);
		if (!objectPtr) {
			continue;
		}

		if (!considerDefaults) {
			uint32_t flags = Read<uint32_t>(objectPtr, 0x08);
			if ((flags & 0x10) != 0) continue;
		}

		uintptr_t initialNameAddress = objectPtr + 0x18;
		uintptr_t classAddress = Read<uintptr_t>(objectPtr, 0x10);

		std::string constructedFullName = GetName(initialNameAddress);

		if (!considerDefaults && constructedFullName.rfind("Default__", 0) == 0) {
			continue;
		}

		if (!considerDefaults && full_name.find('/') == std::string::npos) {
			if (constructedFullName.find(full_name) != std::string::npos) {
				return objectPtr;
			}
		}

		if (constructedFullName != short_name) {
			continue;
		}

		bool moreOuter = true;
		uintptr_t nextOuterAddress = initialNameAddress + 0x08;
		while (moreOuter) {
			uintptr_t outer = Read<uintptr_t>(nextOuterAddress, 0x00);
			if (outer) {
				uintptr_t nameAddress = outer + 0x18;
				constructedFullName = GetName(nameAddress) + "." + constructedFullName;
				nextOuterAddress = nameAddress + 0x08;
				continue;
			}

			moreOuter = false;
			break;
		}

		uintptr_t classNameAddress = classAddress + 0x18;
		std::string class_name = GetName(classNameAddress).c_str();
		if (skipTypeClass && class_name == "Class") 
			continue;

		constructedFullName = class_name + " " + constructedFullName;

		if (constructedFullName == full_name) {
			return objectPtr;
		}
	}

	return 0;
}

uintptr_t GrabObjectAtIndex(int index) {
	if (!objectArray) {
		Print(Error, "Failed to grab object table address.");
		return 0;
	}

	uintptr_t tocPointer = Read<uintptr_t>(objectArray, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return 0;
	}

	int chunk = index / PAGE_MAX;
	int cindex = index % PAGE_MAX;

	uintptr_t chunkPtr = Read<uintptr_t>(tocPointer, chunk * 8);
	if (!chunkPtr) {
		Print(Warning, std::format("Chunk {} is null, skipping object {}", chunk, cindex));
	}

	uintptr_t objectPtr = Read<uintptr_t>(chunkPtr, cindex * 24);
	if (!objectPtr) {
		Print(PrintType::Error, "Failed to find object ptr.");
		return 0;
	}

	return objectPtr;
}

void PrintAllObjects() {

	Print(Info, "Checking GObjects...");

	if (!objectArray) {
		Print(Error, "Failed to grab object table address.");
		return;
	}

	uintptr_t tocPointer = Read<uintptr_t>(objectArray, 0x00);
	if (!tocPointer) {
		Print(Error, "Failed to grab table of contents");
		return;
	}

	std::ofstream dumpFile("C:\\Solutions\\Spicewood\\gobjects_dump.txt", std::ios::out);
	if (!dumpFile.is_open()) {
		Print(Error, "Failed to create dump file path. Check folder permissions!");
		return;
	}

	dumpFile << "Full GObjects Dump \n";

	uint32_t numObjects = gObjectsNum();
	for (int i = 0; i < numObjects; i++) {
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

		std::string full_name = GetName(initialNameAddress);

		bool moreOuter = true;
		uintptr_t nextOuterAddress = initialNameAddress + 0x08;
		while (moreOuter) {
			uintptr_t outer = Read<uintptr_t>(nextOuterAddress, 0x00);
			if (outer) {
				uintptr_t nameAddress = outer + 0x18;
				full_name = GetName(nameAddress) + "." + full_name;
				nextOuterAddress = nameAddress + 0x08;
				continue;
			}

			moreOuter = false;
			break;
		}


		uintptr_t classNameAddress = classAddress + 0x18;
		std::string class_name = GetName(classNameAddress);

		full_name = class_name + " " + full_name;

		// Print(PrintType::Debug, full_name);

		int32_t trueIndex = Read<int32_t>(objectPtr, 0x0c);

		dumpFile << "[" << i << "] (InternalIdx: " << trueIndex << ") "
			<< "Address: 0x" << std::hex << objectPtr << " | "
			<< "ClassPtr: 0x" << classAddress << " | "
			<< "Full: " << full_name << "\n";
	}

	dumpFile.close();
	Print(PrintType::Info, "Dump complete!");
}
