#include "Game.h"
#include "Runtime.h"
#include "Data.h"
#include "EventSystems.h"

namespace Game::Initializers {
	Types::InitializePlayerRandomAgendas orig_InitializePlayerRandomAgendas;
	Types::InitializePlayerRandomAgendas base_InitializePlayerRandomAgendas;
	void InitializePlayerRandomAgendas(int eraIndex, void* agendaDefinitions, int visibilityType, int playerId) {
		using namespace EventSystems;
		using namespace Data;

		if (DoesProcessorExist("InitializePlayerRandomAgendas")) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("EraIndex", LuaVariant(eraIndex));
			variantMap.emplace("DiplomaticVisibilityType", LuaVariant(visibilityType));
			variantMap.emplace("PlayerType", LuaVariant(playerId));

			std::cout << "Calling InitializePlayerRandomAgendas Processor!\n";
			if (CallProcessors("InitializePlayerRandomAgendas", variantMap)) {
				std::cout << "InitializePlayerRandomAgendas returned true!\n";
				return;
			}
			std::cout << "InitializePlayerRandomAgendas returned false!\n";
		}
		else {
			std::cout << "InitializePlayerRandomAgendas Processor not found!\n";
		}

		return base_InitializePlayerRandomAgendas(eraIndex, agendaDefinitions, visibilityType, playerId);
	}

	Types::NewGamePlayerRandomAgendas orig_NewGamePlayerRandomAgendas;
	Types::NewGamePlayerRandomAgendas base_NewGamePlayerRandomAgendas;
	void NewGamePlayerRandomAgendas(Game::Instance* game) {
		std::cout << game << " the game\n";
		base_NewGamePlayerRandomAgendas(game);
	}
}

namespace Game {
	namespace Cache {
		Cache::Types::GetInstance GetInstance;

		namespace Context {
			Types::EditInstance EditInstance;
		}
	}

	namespace hks {
		hks::Types::istable istable;
	}

	Types::FAutoVariable_edit FAutoVariable_edit;
	Types::GetGameplayDatabase GetGameplayDatabase;
	Types::LuaLockAccess LuaLockAccess;
	Types::LuaUnlockAccess LuaUnlockAccess;

	static void PreventHistorialAgendasFromBeingSet() {
		using namespace Runtime;

		byte nops[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteCodeToGameCore(0x227e45, nops, sizeof(nops));
	}

	void Create() {
		using namespace Runtime;

		//PreventHistorialAgendasFromBeingSet();

		hks::istable = GetGameCoreGlobalAt<hks::Types::istable>(hks::IS_TABLE_OFFSET);

		LuaLockAccess = GetGameCoreGlobalAt<Types::LuaLockAccess>(LUA_LOCK_ACCESS_OFFSET);
		LuaUnlockAccess = GetGameCoreGlobalAt<Types::LuaUnlockAccess>(LUA_UNLOCK_ACCESS_OFFSET);

		FAutoVariable_edit = GetGameCoreGlobalAt<Types::FAutoVariable_edit>(F_AUTO_VARIABLE_EDIT_OFFSET);
		GetGameplayDatabase = GetGameCoreGlobalAt<Types::GetGameplayDatabase>(GET_GAMEPLAY_DATABASE_OFFSET);

		Cache::GetInstance = GetGameCoreGlobalAt<Cache::Types::GetInstance>(Cache::GET_INSTANCE_OFFSET);
		Cache::Context::EditInstance = GetGameCoreGlobalAt<Cache::Context::Types::EditInstance>(Cache::Context::EDIT_INSTANCE_OFFSET);

		Initializers::orig_InitializePlayerRandomAgendas =
			GetGameCoreGlobalAt<Initializers::Types::InitializePlayerRandomAgendas>(Initializers::INITIALIZE_PLAYER_RANDOM_AGENDAS_OFFSET);
		CreateHook(Initializers::orig_InitializePlayerRandomAgendas, &Initializers::InitializePlayerRandomAgendas, &Initializers::base_InitializePlayerRandomAgendas);

		Initializers::orig_NewGamePlayerRandomAgendas =
			GetGameCoreGlobalAt<Initializers::Types::NewGamePlayerRandomAgendas>(Initializers::NEW_GAME_PLAYER_RANDOM_AGENDAS_OFFSET);
		CreateHook(Initializers::orig_NewGamePlayerRandomAgendas, &Initializers::NewGamePlayerRandomAgendas, &Initializers::base_NewGamePlayerRandomAgendas);
	}
}
