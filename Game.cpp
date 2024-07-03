#include "Game.h"
#include "Runtime.h"

namespace Game {
	Types::FAutoVariable_edit FAutoVariable_edit;

	void Create() {
		using namespace Runtime;

		FAutoVariable_edit = GetGameCoreGlobalAt<Types::FAutoVariable_edit>(F_AUTO_VARIABLE_EDIT_OFFSET);
	}
}