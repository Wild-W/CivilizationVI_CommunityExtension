#pragma once
#include "MinHook.h"
#include "ProxyTypes.h"
#include <iostream>
#include "capstone/capstone.h"
#include "Log.h"

namespace Runtime {
	extern HMODULE GameCore;
	extern uintptr_t GameCoreAddress;
	extern DWORD GameProcessId;
	extern HANDLE GameProcess;
	extern HMODULE CivVIModHandle;

	extern Log::Logger* MainLogger;

	namespace Base {
		using EXP_GetLogPath_t = size_t(__fastcall*)(wchar_t* pszOutBuffer, const wchar_t* pszSubPath);
		extern EXP_GetLogPath_t EXP_GetLogPath;

		using EnterLuaCriticalSection_t = void(*)(void);
		extern EnterLuaCriticalSection_t EnterLuaCriticalSection;

		using LeaveLuaCriticalSection_t = void(*)(void);
		extern LeaveLuaCriticalSection_t LeaveLuaCriticalSection;

		extern void Create();
	}

	// Should only be called once
	extern void Create();

	// Call on dll exit
	extern void Destroy();

	extern void InitMinHook();

	template <typename T>
	T GetGameCoreGlobalAt(uintptr_t address) {
		return reinterpret_cast<T>(GameCoreAddress + address);
	}

	template <typename T>
	T GetGlobalAt(uintptr_t address) {
		return reinterpret_cast<T>((uintptr_t)CivVIModHandle + address);
	}

	template <typename T>
	void CreateHook(LPVOID pTarget, LPVOID pDetour, T** ppOriginal) {
		MH_STATUS status;
		if ((status = MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal))) != MH_OK) {
			Runtime::MainLogger->Error("Failed to load hook! {}", MH_StatusToString(status));
		}
		if ((status = MH_EnableHook(pTarget)) != MH_OK) {
			Runtime::MainLogger->Info("Failed to enable hook! {}", MH_StatusToString(status));
		}
	}

	extern void InitConsole();
	extern void CloseConsole();

	extern BOOL WriteCodeToGameCore(uintptr_t address, byte* bytes, size_t sizeOfBytes);
	extern BOOL WriteCodeToProcess(uintptr_t address, byte* bytes, size_t sizeOfBytes);
};