#include "Unit.h"
#include "Runtime.h"

namespace Unit {
	Types::GetInstance GetInstance;
	Types::Push Push;

	void Create() {
		using namespace Runtime;

		Push = GetGameCoreGlobalAt<Types::Push>(PUSH_OFFSET);
		GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
	}
}