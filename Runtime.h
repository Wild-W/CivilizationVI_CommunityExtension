#pragma once
#include "MinHook.h"
#include "ProxyTypes.h"
#include "asmjit/x86.h"
#include <iostream>

namespace Runtime {
	extern HMODULE GameCore;
	extern uintptr_t GameCoreAddress;
	extern asmjit::JitRuntime Jit;

	// Should only be called once
	extern void Create();

	// Call on dll exit
	extern void Destroy();

	void InitMinHook();

	template <typename T>
	T GetGameCoreGlobalAt(uintptr_t address) {
		return reinterpret_cast<T>(GameCoreAddress + address);
	}

	template <typename Target, typename Detour, typename T>
	void CreateHook(Target pTarget, Detour pDetour, T** ppOriginal) {
		MH_STATUS status;
		if ((status = MH_CreateHook(reinterpret_cast<LPVOID>(pTarget), reinterpret_cast<LPVOID>(pDetour), reinterpret_cast<LPVOID*>(ppOriginal))) != MH_OK) {
			std::cout << "Failed to load hook! " << MH_StatusToString(status) << "\n";
		}
		if ((status = MH_EnableHook(reinterpret_cast<LPVOID>(pTarget))) != MH_OK) {
			std::cout << "Failed to enable hook! " << MH_StatusToString(status) << "\n";
		}
	}

	extern void InitConsole();

	extern void CloseConsole();
};