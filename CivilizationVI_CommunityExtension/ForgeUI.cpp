#include "ForgeUI.h"
#include "Runtime.h"

namespace ForgeUI {
	Types::GetLock GetLock;
	Types::ReleaseLock ReleaseLock;
	ForgeUI_UIManager* manager;

	void Create() {
		using namespace Runtime;

		GetLock = GetGlobalAt<Types::GetLock>(GET_LOCK_OFFSET);
		ReleaseLock = GetGlobalAt<Types::ReleaseLock>(RELEASE_LOCK_OFFSET);
		manager = GetGlobalAt<ForgeUI_UIManager*>(MANAGER_OFFSET);
	}
}