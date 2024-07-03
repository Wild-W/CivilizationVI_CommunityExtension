#pragma once
#include "Runtime.h"

namespace Game {
	namespace Types {
		typedef void* (__thiscall* FAutoVariable_edit)(void* object);
	}

	extern Types::FAutoVariable_edit FAutoVariable_edit;

	constexpr uintptr_t F_AUTO_VARIABLE_EDIT_OFFSET = 0x72a920;

	extern void Create();
}