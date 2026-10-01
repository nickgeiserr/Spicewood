#include "pch.h"
#include "Core/Console.h"
#include <cstdio>
#include <iostream>
#include <mutex>

static const char* VERSION = "v0.1.0";
static std::mutex consoleMutex;

static int ConsoleWidth() {
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		return info.srWindow.Right - info.srWindow.Left + 1;
	return 80;
}

void CreateConsole() {
	if (AllocConsole()) {
		FILE* fDummy;

		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
	}

	SetConsoleTitleA("Spicewood");
	SetConsoleOutputCP(CP_UTF8);

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(consoleHandle, &mode);
	SetConsoleMode(consoleHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	std::cout << Mocha::Base << "\x1b[2J\x1b[3J\x1b[H\x1b[?25l" << std::flush;

	DrawHeader();
}

void CleanupConsole() {
	std::cout << "\x1b[0m\x1b[?25h" << std::flush;
	FreeConsole();
}

void DrawHeader() {
	std::lock_guard<std::mutex> lock(consoleMutex);
	int width = ConsoleWidth() - 4;

	std::string title = "  SPICEWOOD  ";
	std::string subtitle = "  Minecraft Dungeons II Mod Framework";
	std::string version = std::string(VERSION) + "  ";

	int gap = width - (int)(title.size() + subtitle.size() + version.size());
	if (gap < 1) gap = 1;

	std::cout << "\n";
	std::cout << "  " << Mocha::BgMauve << Mocha::Crust << Mocha::Bold << title
		<< Mocha::Base << Mocha::BgSurface0 << Mocha::Text << subtitle
		<< std::string(gap, ' ')
		<< Mocha::Overlay << version
		<< Mocha::Base << "\n\n";
}

void DrawKeybinds(std::initializer_list<std::pair<const char*, const char*>> keys) {
	std::lock_guard<std::mutex> lock(consoleMutex);
	std::cout << "  ";
	for (auto& key : keys) {
		std::cout << Mocha::BgSurface0 << Mocha::Lavender << Mocha::Bold << " " << key.first << " "
			<< Mocha::Base << " " << Mocha::Subtext << key.second << "    ";
	}
	std::cout << Mocha::Base << "\n\n";

	std::cout << "  " << Mocha::Surface;
	for (int i = 0; i < ConsoleWidth() - 4; i++)
		std::cout << "─";
	std::cout << Mocha::Base << "\n\n" << std::flush;
}

static std::string Timestamp() {
	SYSTEMTIME t;
	GetLocalTime(&t);
	char time[16];
	snprintf(time, sizeof(time), "%02d:%02d:%02d", t.wHour, t.wMinute, t.wSecond);
	return time;
}

std::string ReadLine(const std::string& prompt) {
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

	{
		std::lock_guard<std::mutex> lock(consoleMutex);
		std::cout << "  " << Mocha::Overlay << Timestamp() << "  "
			<< Mocha::BgLavender << Mocha::Crust << Mocha::Bold << " INPUT "
			<< Mocha::Base << "  " << prompt;
		if (!prompt.empty())
			std::cout << " ";
		std::cout << Mocha::Lavender << "› " << Mocha::Text << "\x1b[?25h" << std::flush;
	}

	std::string line;
	if (!std::getline(std::cin, line))
		std::cin.clear();

	std::lock_guard<std::mutex> lock(consoleMutex);
	std::cout << Mocha::Base << "\x1b[?25l" << std::flush;
	return line;
}

void Print(PrintType type, const std::string& message) {
	const char* bg = Mocha::BgBlue;
	const char* label = " INFO  ";

	switch (type) {
		case Startup: bg = Mocha::BgGreen;  label = " OK    "; break;
		case Info:    bg = Mocha::BgBlue;   label = " INFO  "; break;
		case Debug:   bg = Mocha::BgMauve;  label = " DEBUG "; break;
		case Warning: bg = Mocha::BgYellow; label = " WARN  "; break;
		case Error:   bg = Mocha::BgRed;    label = " ERROR "; break;
	}

	std::string time = Timestamp();

	std::lock_guard<std::mutex> lock(consoleMutex);
	std::cout << "  " << Mocha::Overlay << time << "  "
		<< bg << Mocha::Crust << Mocha::Bold << label
		<< Mocha::Base << "  " << message << "\n" << std::flush;
}
