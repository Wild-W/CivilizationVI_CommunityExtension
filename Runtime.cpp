#include "Runtime.h"
#include "MinHook.h"
#include <iostream>
#include "HavokScript.h"
#pragma comment(lib, "libMinHook.x64.lib")

namespace Runtime {
	HMODULE GameCore;
	uintptr_t GameCoreAddress;
	asmjit::JitRuntime Jit;
	HANDLE GameProcess;

	// Should only be called once
	void Create() {
		GameCore = LoadLibrary(TEXT("../../../DLC/Expansion2/Binaries/Win64/GameCore_XP2_FinalRelease.dll"));
		if (!GameCore) {
			std::cout << "Original GameCore failed to load!\n";
			return;
		}
		GameCoreAddress = reinterpret_cast<uintptr_t>(GameCore);
		GameProcess = GetCurrentProcess();
	}

	void InitMinHook() {
		if (MH_Initialize() != MH_OK) {
			std::cout << "MH failed to init\n";
		}
		std::cout << "MH initialized successfully.\n";
	}

	// Call on dll exit
	void Destroy() {
		if (GameCore) {
			MH_DisableHook(MH_ALL_HOOKS);
			MH_Uninitialize();
			FreeLibrary(GameCore);
			GameCore = NULL;
			GameCoreAddress = 0x0;
		}
	}

	void InitConsole() {
		if (!AllocConsole()) {
			MessageBoxW(NULL, L"Failed to create the console!", L"Error", MB_ICONERROR);
			return;
		}

		// Redirect standard input/output streams to the console
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

		std::cout << "Console initialized successfully." << std::endl;
	}

	void CloseConsole() {
		if (!FreeConsole()) {
			MessageBoxW(NULL, L"Failed to close the console!", L"Error", MB_ICONERROR);
		}
		else {
			std::cout << "Console closed successfully." << std::endl;
		}

		// Reset standard input/output streams
		freopen_s((FILE**)stdin, "NUL:", "r", stdin);
		freopen_s((FILE**)stdout, "NUL:", "w", stdout);
		freopen_s((FILE**)stderr, "NUL:", "w", stderr);

		// Clear the error state for each of the C++ standard streams after redirect.
		std::cin.clear();
		std::cout.clear();
		std::cerr.clear();
	}

	BOOL WriteCodeToProcess(uintptr_t address, byte* bytes, size_t sizeOfBytes) {
		DWORD oldProtect;
		LPVOID lpBaseAddress = reinterpret_cast<LPVOID>(address);

		if (!VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, PAGE_EXECUTE_READWRITE, &oldProtect)) {
			std::cerr << "Failed to change memory protection: " << GetLastError() << std::endl;
			return FALSE;
		}

		SIZE_T numberOfBytesWritten;
		BOOL result = WriteProcessMemory(GameProcess, lpBaseAddress, bytes, sizeOfBytes, &numberOfBytesWritten);

		if (!result || numberOfBytesWritten != sizeOfBytes) {
			std::cerr << "Failed to write to memory: " << GetLastError() << std::endl;
			VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, oldProtect, &oldProtect);
			return FALSE;
		}

		VirtualProtectEx(GameProcess, lpBaseAddress, sizeOfBytes, oldProtect, &oldProtect);

		return TRUE;
	}

	BOOL WriteCodeToGameCore(uintptr_t address, byte* bytes, size_t sizeOfBytes) {
		return WriteCodeToProcess(address + GameCoreAddress, bytes, sizeOfBytes);
	}
};
