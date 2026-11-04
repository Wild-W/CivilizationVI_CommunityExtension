#include "Runtime.h"
#include "MinHook.h"
#include <iostream>
#include "HavokScript.h"
#pragma comment(lib, "libMinHook.x64.lib")

namespace Runtime {
	HMODULE GameCore;
	uintptr_t GameCoreAddress;
	DWORD GameProcessId;
	HANDLE GameProcess;
	HMODULE CivVIModHandle;

	Log::Logger* MainLogger;

	namespace Base {
		EXP_GetLogPath_t EXP_GetLogPath;

		EnterLuaCriticalSection_t EnterLuaCriticalSection;
		LeaveLuaCriticalSection_t LeaveLuaCriticalSection;

		void Create() {
			EXP_GetLogPath = (EXP_GetLogPath_t)GetProcAddress(CivVIModHandle, "EXP_GetLogPath");

			EnterLuaCriticalSection = GetGlobalAt<EnterLuaCriticalSection_t>(0xdafa40);
			LeaveLuaCriticalSection = GetGlobalAt<LeaveLuaCriticalSection_t>(0xdb0660);
		}
	}

	// Should only be called once
	void Create() {
		// Prologue; gather all program data
		GameProcessId = GetCurrentProcessId();
		GameProcess = GetCurrentProcess();
		CivVIModHandle = GetModuleHandle(NULL);

		// Create base runtime references
		Base::Create();

		// Get the log directory
		std::wstring buffer(MAX_PATH, L'\0');
		size_t charsWritten = Base::EXP_GetLogPath(buffer.data(), L"CE/");
		buffer.resize(charsWritten);
		std::filesystem::path logPath(buffer);

		// Create a CE/ subdirectory if it doesn't exist
		std::error_code ec;
		std::filesystem::create_directories(logPath, ec);

		// Initialize the logger library
		Log::Init(logPath);

		// Default main logger. Specialized Loggers can be created elsewhere.
		MainLogger = new Log::Logger("CE", "Community_Extension.log");

		// Load GameCore
		GameCore = LoadLibrary(TEXT("../../../DLC/Expansion2/Binaries/Win64/GameCore_XP2_FinalRelease.dll"));
		if (!GameCore) {
			Runtime::MainLogger->Error("DLC/Expansion2/Binaries/Win64/GameCore_XP2_FinalRelease.dll failed to load!");
			return;
		}
		GameCoreAddress = reinterpret_cast<uintptr_t>(GameCore);
	}

	void InitMinHook() {
		if (MH_Initialize() != MH_OK) {
			Runtime::MainLogger->Info("MH failed to init");
		}
		Runtime::MainLogger->Info("MH initialized successfully.");
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

		// Enable ANSI color escape sequence processing
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		if (GetConsoleMode(hOut, &dwMode)) {
			SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}

		// Redirect standard input/output streams to the console
		FILE* fDummy;
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONOUT$", "w", stdout);

		MainLogger->Info("Console initialized!");
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
			MainLogger->Error("Failed to change memory protection: {}", GetLastError());
			return FALSE;
		}

		SIZE_T numberOfBytesWritten;
		BOOL result = WriteProcessMemory(GameProcess, lpBaseAddress, bytes, sizeOfBytes, &numberOfBytesWritten);

		if (!result || numberOfBytesWritten != sizeOfBytes) {
			MainLogger->Error("Failed to write to memory: {}", GetLastError());
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
