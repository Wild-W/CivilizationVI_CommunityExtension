#pragma once
#include "HavokScript.h"

namespace ForgeUI {
	class ForgeUI_UIManager;
	namespace Types {
		typedef void (*GetLock)(ForgeUI_UIManager*);
		typedef void (*ReleaseLock)(ForgeUI_UIManager*);
	}
	
	constexpr uintptr_t GET_LOCK_OFFSET = 0x88b330;
	extern Types::GetLock GetLock;

	constexpr uintptr_t RELEASE_LOCK_OFFSET = 0x8948a0;
	extern Types::ReleaseLock ReleaseLock;

	constexpr uintptr_t MANAGER_OFFSET = 0x235f7c8;
	extern ForgeUI_UIManager* manager;

	extern void Create();
}