#pragma once

#include <string>
#include <sstream>
#include <istream>
#include <utility>
#include <type_traits>
#include <initializer_list>

namespace Mocha {
	constexpr const char* Base     = "\x1b[0m\x1b[48;2;30;30;46m\x1b[38;2;205;214;244m";

	constexpr const char* Text     = "\x1b[38;2;205;214;244m";
	constexpr const char* Subtext  = "\x1b[38;2;166;173;200m";
	constexpr const char* Overlay  = "\x1b[38;2;108;112;134m";
	constexpr const char* Surface  = "\x1b[38;2;69;71;90m";
	constexpr const char* Crust    = "\x1b[38;2;17;17;27m";

	constexpr const char* Green    = "\x1b[38;2;166;227;161m";
	constexpr const char* Blue     = "\x1b[38;2;137;180;250m";
	constexpr const char* Mauve    = "\x1b[38;2;203;166;247m";
	constexpr const char* Yellow   = "\x1b[38;2;249;226;175m";
	constexpr const char* Red      = "\x1b[38;2;243;139;168m";
	constexpr const char* Peach    = "\x1b[38;2;250;179;135m";
	constexpr const char* Lavender = "\x1b[38;2;180;190;254m";

	constexpr const char* BgSurface0 = "\x1b[48;2;49;50;68m";
	constexpr const char* BgSurface1 = "\x1b[48;2;69;71;90m";
	constexpr const char* BgGreen    = "\x1b[48;2;166;227;161m";
	constexpr const char* BgBlue     = "\x1b[48;2;137;180;250m";
	constexpr const char* BgMauve    = "\x1b[48;2;203;166;247m";
	constexpr const char* BgYellow   = "\x1b[48;2;249;226;175m";
	constexpr const char* BgRed      = "\x1b[48;2;243;139;168m";
	constexpr const char* BgLavender = "\x1b[48;2;180;190;254m";

	constexpr const char* Bold     = "\x1b[1m";
}

enum PrintType {
	// Used once at startup for a special message
	Startup,
	// Used to print normal mod information / output
	Info,
	// Used specifically for debugging purposes. Will be hidden if DebugMode = false
	Debug,
	// Used for printing errors
	Error,
	// Used for printing a warning. Ex. x didn't load properly so y might not work but some parts might
	Warning
};

// Set & Get Console Debug Mode
void SetDebugMode(bool dm);
bool IsDebugMode();

// Create Console Pipeline
void CreateConsole();

// Cleanup Console
void CleanupConsole();

void DrawHeader();
void DrawKeybinds(std::initializer_list<std::pair<const char*, const char*>> keys);

void Print(PrintType type, const std::string& message);

std::string ReadLine(const std::string& prompt);

template<typename T = std::string>
T Input(const std::string& prompt = "") {
	while (true) {
		std::string line = ReadLine(prompt);

		if constexpr (std::is_same_v<T, std::string>) {
			return line;
		}
		else {
			std::istringstream stream(line);
			T value{};
			if (stream >> value && (stream >> std::ws).eof())
				return value;

			Print(Error, "\"" + line + "\" isn't valid, try again");
		}
	}
}
