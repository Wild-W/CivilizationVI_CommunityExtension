#include "MinHook.h"
#include <iostream>
#pragma comment(lib, "libMinHook.x64.lib")

namespace Runtime {
	HMODULE GameCore;
	uintptr_t GameCoreAddress;

	// Should only be called once
	void Create() {
		GameCore = LoadLibrary(TEXT("proxy.dll"));
		if (!GameCore) {
			std::cout << "Original GameCore failed to load!\n";
		}
		GameCoreAddress = reinterpret_cast<uintptr_t>(GameCore);
	}

	void InitHooks() {
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
		}
	}
};
