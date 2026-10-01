#include "pch.h"
#include "Core/Strings.h"

std::string ToUtf8(const std::wstring& wide) {
	if (wide.empty()) return "";

	int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(),
		nullptr, 0, nullptr, nullptr);     

	std::string result(size, '\0');                                   
	WideCharToMultiByte(CP_UTF8, 0, wide.data(), (int)wide.size(),
		result.data(), size, nullptr, nullptr);          
	return result;
}
