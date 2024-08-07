#pragma once
#include "Runtime.h"

namespace Game::Initializers {
	namespace Types {
		typedef void(__cdecl* InitializePlayerRandomAgendas)(int somethingToDoWithEra, void* agendaDefinitions, int visibilityType, int playerId);
	}

	constexpr uintptr_t INITIALIZE_PLAYER_RANDOM_AGENDAS_OFFSET = 0x228800;
	extern void InitializePlayerRandomAgendas(int somethingToDoWithEra, void* agendaDefinitions, int visibilityType, int playerId);
}

namespace Game {
	namespace Cache {
		typedef class Game;
		namespace Types {
			typedef Cache::Game* (__cdecl* GetInstance)(void);
		}

		constexpr uintptr_t GET_INSTANCE_OFFSET = 0x7aeb0;
		extern Cache::Types::GetInstance GetInstance;
	}
	namespace Types {
		typedef void* (__thiscall* FAutoVariable_edit)(void* object);
	}

	constexpr uintptr_t F_AUTO_VARIABLE_EDIT_OFFSET = 0x72a920;
	extern Types::FAutoVariable_edit FAutoVariable_edit;

	extern void Create();
}

