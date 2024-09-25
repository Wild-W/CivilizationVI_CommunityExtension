#pragma once
#include "Runtime.h"
#include "HavokScript.h"

namespace Game {
	typedef class IDatabaseConnection1;
	typedef class Instance;
	namespace Cache {
		typedef class Game;
		namespace Types {
			typedef Cache::Game* (__cdecl* GetInstance)(void);
		}

		namespace Context {
			typedef class Instance;
			namespace Types {
				typedef Cache::Context::Instance* (__cdecl* EditInstance)(void);
			}

			constexpr uintptr_t EDIT_INSTANCE_OFFSET = 0xa200;
			extern Types::EditInstance EditInstance;
		}

		constexpr uintptr_t GET_INSTANCE_OFFSET = 0x7aeb0;
		extern Cache::Types::GetInstance GetInstance;
	}
	namespace Types {
		typedef void* (__thiscall* FAutoVariable_edit)(void* object);
		typedef IDatabaseConnection1* (__cdecl* GetGameplayDatabase)(void);
		typedef void(__cdecl* LuaLockAccess)(void);
		typedef void(__cdecl* LuaUnlockAccess)(void);
	}
	namespace hks {
		namespace Types {
			typedef int (__cdecl* istable)(::hks::lua_State* L, int pos);
		}

		constexpr uintptr_t IS_TABLE_OFFSET = 0x221c0;
		extern Types::istable istable;
	}

	constexpr uintptr_t F_AUTO_VARIABLE_EDIT_OFFSET = 0x72a920;
	extern Types::FAutoVariable_edit FAutoVariable_edit;

	constexpr uintptr_t GET_GAMEPLAY_DATABASE_OFFSET = 0x44de0;
	extern Types::GetGameplayDatabase GetGameplayDatabase;

	constexpr uintptr_t LUA_LOCK_ACCESS_OFFSET = 0x9adf70;
	extern Types::LuaLockAccess LuaLockAccess;

	constexpr uintptr_t LUA_UNLOCK_ACCESS_OFFSET = 0x9ae060;
	extern Types::LuaUnlockAccess LuaUnlockAccess;

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
