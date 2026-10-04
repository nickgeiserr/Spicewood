#include <iostream>
#include <Windows.h>

int main() {

	bool try_inject_start = false;
	bool dump_game = false;
	LPCSTR dll = "";
	if (dump_game)
		dll = R"(C:\Solutions\Spicewood\SpicewoodLibrary\x64\Release\dumper-7.dll)";
	else
		dll = R"(C:\Solutions\Spicewood\SpicewoodLibrary\x64\Release\Spicewood.dll)";

	std::cout << "------- Spicewood Injector -------" << std::endl;

	HWND windowHandle = NULL;

	if (try_inject_start) {
		while (true) {
			std::cout << "Attempting to find handle to Minecraft Dungeons II" << std::endl;
			HWND handle = FindWindowA("UnrealWindow", NULL);

			if (handle == NULL) {
				continue;
			}

			std::cout << "Window Handle found!" << std::endl;
			windowHandle = handle;
			break;
		}
	}
	else {
		std::cout << "Attempting to find handle to Minecraft Dungeons II" << std::endl;
		windowHandle = FindWindowA("UnrealWindow", NULL);

		if (windowHandle == NULL) {
			std::cout << "Failed to find the game window. Is it running? Try Injecting Again" << std::endl;
			return 0;
		}

		std::cout << "Window Handle found!" << std::endl;
	}


	DWORD proc_id = 0;
	GetWindowThreadProcessId(windowHandle, &proc_id);

	if (!proc_id) {
		std::cout << "Failed to grab process ID. Are you sure the game is running? Try Injecting again" << std::endl;
		return 0;
	}

	std::cout << "Process ID Found: " << proc_id << std::endl;

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc_id);
	if (handle == nullptr) {
		std::cerr << "OpenProcess failed: "
			<< GetLastError() << '\n';
		return 1;
	}

	LPVOID allocDllPath = VirtualAllocEx(handle, 0, strlen(dll) + 1, MEM_COMMIT, PAGE_READWRITE);

	WriteProcessMemory(handle, allocDllPath, (LPVOID)dll, strlen(dll) + 1, 0);

	HMODULE kernel32 = GetModuleHandleA("Kernel32.dll");
	if (kernel32 == NULL) {
		std::cout << "Failed to grab kernel32 handle. Try running injector again. " << std::endl;
		return 0;
	}

	std::cout << "Grabbed kernel32 handle. " << std::endl;

	FARPROC procAddress = GetProcAddress(kernel32, "LoadLibraryA");
	if (procAddress == NULL) {
		std::cout << "Failed to grab process address of LoadLibraryA. " << std::endl;
		return 0;
	}

	std::cout << "Found LoadLibraryA! " << std::endl;

	HANDLE hRemoteLoad = CreateRemoteThread(handle, 0, 0,
		(LPTHREAD_START_ROUTINE)procAddress,
		allocDllPath, 0, 0);

	if (hRemoteLoad == nullptr) {
		std::cerr << "CreateRemoteThread failed: "
			<< GetLastError() << '\n';
		return 1;
	}

	DWORD waitResult = WaitForSingleObject(hRemoteLoad, INFINITE);

	if (waitResult != WAIT_OBJECT_0) {
		std::cerr << "WaitForSingleObject failed: "
			<< GetLastError() << '\n';
	}

	CloseHandle(hRemoteLoad);

	std::cout << "Spicewood Injected. Input to close" << std::endl;
	std::cin.get();

	std::cout << "Bye bye! Freeing memory now";

	if (allocDllPath != nullptr) {
		if (!VirtualFreeEx(handle, allocDllPath, 0, MEM_RELEASE)) {
			DWORD err = GetLastError();
			std::cout << "Failed to release memory: " << err << std::endl;
		}
		allocDllPath = nullptr;
	}

	return 0;

}