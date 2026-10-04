#include "pch.h"
#include "Core/Console.h"
#include <windows.h>
#include <string>
#include <sstream>

static HANDLE hPipeConnection = INVALID_HANDLE_VALUE;

static std::string Timestamp() {
    SYSTEMTIME t;
    GetLocalTime(&t);
    char time[16];
    snprintf(time, sizeof(time), "%02d:%02d:%02d", t.wHour, t.wMinute, t.wSecond);
    return std::string(time);
}

void CreateConsole() {
    while (hPipeConnection == INVALID_HANDLE_VALUE) {
        hPipeConnection = CreateFileA(
            "\\\\.\\pipe\\spicewood_pipeline",
            GENERIC_READ | GENERIC_WRITE,
            0, NULL, OPEN_EXISTING, 0, NULL
        );

        if (hPipeConnection == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();

            if (error == ERROR_PIPE_BUSY) {
                WaitNamedPipeA("\\\\.\\pipe\\spicewood_pipeline", 2000);
                continue;
            }

            Sleep(100);
        }
    }
}

void CleanupConsole() {
    if (hPipeConnection != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        std::string signal = "TRIGGER_CLEAN_UNLOAD";
        WriteFile(hPipeConnection, signal.c_str(), (DWORD)signal.length(), &bytesWritten, NULL);
        CloseHandle(hPipeConnection);
        hPipeConnection = INVALID_HANDLE_VALUE;
    }
}

void DrawHeader() {}
void DrawKeybinds(std::initializer_list<std::pair<const char*, const char*>> keys) {}

void Print(PrintType type, const std::string& message) {
    if (hPipeConnection == INVALID_HANDLE_VALUE) return;

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
    std::stringstream ss;

    ss << "  " << Mocha::Overlay << time << "  "
        << bg << Mocha::Crust << Mocha::Bold << label
        << Mocha::Base << "  " << message << "\n";

    std::string formattedMsg = ss.str();
    DWORD bytesWritten;
    WriteFile(hPipeConnection, formattedMsg.c_str(), (DWORD)formattedMsg.length(), &bytesWritten, NULL);
}

std::string ReadLine(const std::string& prompt) {
    if (hPipeConnection == INVALID_HANDLE_VALUE) return "";

    std::string request = "REQ_INPUT:" + prompt;
    DWORD bytesWritten;
    WriteFile(hPipeConnection, request.c_str(), (DWORD)request.length(), &bytesWritten, NULL);

    char replyBuffer[1024];
    ZeroMemory(replyBuffer, sizeof(replyBuffer));
    DWORD bytesRead = 0;

    if (ReadFile(hPipeConnection, replyBuffer, sizeof(replyBuffer) - 1, &bytesRead, NULL)) {
        if (bytesRead > 0) {
            replyBuffer[bytesRead] = '\0';
            return std::string(replyBuffer);
        }
    }

    return "";
}
