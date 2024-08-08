#pragma once
#include "Runtime.h"

namespace Game {
	typedef class IDatabaseConnection1;
	typedef class Instance;
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
		typedef IDatabaseConnection1* (__cdecl* GetGameplayDatabase)(void);
	}

	constexpr uintptr_t F_AUTO_VARIABLE_EDIT_OFFSET = 0x72a920;
	extern Types::FAutoVariable_edit FAutoVariable_edit;

	constexpr uintptr_t GET_GAMEPLAY_DATABASE_OFFSET = 0x44de0;
	extern Types::GetGameplayDatabase GetGameplayDatabase;

	extern void Create();
}

namespace Game::Initializers {
	namespace Types {
		typedef void(__cdecl* InitializePlayerRandomAgendas)(int somethingToDoWithEra, void* agendaDefinitions, int visibilityType, int playerId);
		typedef void(__cdecl* NewGamePlayerRandomAgendas)(Game::Instance* game);
	}

	constexpr uintptr_t INITIALIZE_PLAYER_RANDOM_AGENDAS_OFFSET = 0x228800;
	extern void InitializePlayerRandomAgendas(int somethingToDoWithEra, void* agendaDefinitions, int visibilityType, int playerId);

	constexpr uintptr_t NEW_GAME_PLAYER_RANDOM_AGENDAS_OFFSET = 0x228750;
	extern void NewGamePlayerRandomAgendas(Game::Instance* game);
}
