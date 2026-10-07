#include <windows.h>
#include <iostream>
#include <string>
#include <mutex>
#include <sstream>
#include <initializer_list>
#include <utility>

namespace Mocha {
    constexpr const char* Base = "\x1b[0m\x1b[48;2;30;30;46m\x1b[38;2;205;214;244m";
    constexpr const char* Text = "\x1b[38;2;205;214;244m";
    constexpr const char* Subtext = "\x1b[38;2;166;173;200m";
    constexpr const char* Overlay = "\x1b[38;2;108;112;134m";
    constexpr const char* Surface = "\x1b[38;2;69;71;90m";
    constexpr const char* Crust = "\x1b[38;2;17;17;27m";

    constexpr const char* Green = "\x1b[38;2;166;227;161m";
    constexpr const char* Blue = "\x1b[38;2;137;180;250m";
    constexpr const char* Mauve = "\x1b[38;2;203;166;247m";
    constexpr const char* Yellow = "\x1b[38;2;249;226;175m";
    constexpr const char* Red = "\x1b[38;2;243;139;168m";
    constexpr const char* Peach = "\x1b[38;2;250;179;135m";
    constexpr const char* Lavender = "\x1b[38;2;180;190;254m";

    constexpr const char* BgSurface0 = "\x1b[48;2;49;50;68m";
    constexpr const char* BgSurface1 = "\x1b[48;2;69;71;90m";
    constexpr const char* BgGreen = "\x1b[48;2;166;227;161m";
    constexpr const char* BgBlue = "\x1b[48;2;137;180;250m";
    constexpr const char* BgMauve = "\x1b[48;2;203;166;247m";
    constexpr const char* BgYellow = "\x1b[48;2;249;226;175m";
    constexpr const char* BgRed = "\x1b[48;2;243;139;168m";
    constexpr const char* BgLavender = "\x1b[48;2;180;190;254m";

    constexpr const char* Bold = "\x1b[1m";
}

enum PrintType {
    Startup,
    Info,
    Debug,
    Error,
    Warning
};

static const char* VERSION = "v0.1.0";
static std::mutex consoleMutex;

static int ConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return info.srWindow.Right - info.srWindow.Left + 1;
    return 80;
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

void CreateConsole() {
    SetConsoleTitleA("Spicewood Server");
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

int main() {
    std::cout << "[SERVER] Starting...\n" << std::flush;
    HANDLE hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\spicewood_pipeline",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1, 65536, 65536, 0, NULL
    );

    if (hPipe == INVALID_HANDLE_VALUE) {
        return 1;
    }

    bool keepRunning = true;
    bool systemInitialized = false;

    while (keepRunning) {
        if (!systemInitialized) {
            std::cout << "Waiting for game connection...\n";
            if (ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED)) {
                CreateConsole();
                DrawKeybinds({
                    {"F9", "Object count"},
                    {"F8", "Check objects"},
                    {"F7", "FindObject"},
                    {"F6", "FindObjectsByClass"},
                    {"F10", "Unload"}
                    });
                systemInitialized = true;
            }
            else {
                Sleep(100);
                continue;
            }
        }

        char buffer[4096];
        DWORD bytesRead;

        if (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                std::string packet(buffer);

                if (packet.rfind("REQ_INPUT:", 0) == 0) {
                    std::string prompt = packet.substr(10);
                    std::string userInput = ReadLine(prompt);

                    DWORD bytesWritten;
                    WriteFile(hPipe, userInput.c_str(), (DWORD)userInput.length(), &bytesWritten, NULL);
                }
                else if (packet == "TRIGGER_CLEAN_UNLOAD") {
                    std::cout << "\n"
                        << Mocha::Red
                        << "[DLL] CLEAN UNLOAD RECEIVED"
                        << Mocha::Base
                        << "\n"
                        << std::flush;

                    keepRunning = true;
                }
                else {
                    std::cout << packet << std::flush;
                }
            }
        }
        else {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
                DisconnectNamedPipe(hPipe);
                systemInitialized = false;
                std::cout << "\n" << Mocha::Red << "[Pipe Closed/Broken] Re-listening..." << Mocha::Base << "\n\n";
            }
        }
        Sleep(1);
    }

    CloseHandle(hPipe);
    CleanupConsole();
    return 0;
}
