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
			if (CallCustomProcessor("InitializePlayerRandomAgendas", variantMap)) {
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
		//base_NewGamePlayerRandomAgendas(game);
	}
}

namespace Game {
	namespace Cache {
		Cache::Types::GetInstance GetInstance;
	}
	Types::FAutoVariable_edit FAutoVariable_edit;
	Types::GetGameplayDatabase GetGameplayDatabase;

	static void PreventHistorialAgendasFromBeingSet() {
		using namespace Runtime;

		byte nops[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteCodeToGameCore(0x227e45, nops, sizeof(nops));
	}

	void Create() {
		using namespace Runtime;

		PreventHistorialAgendasFromBeingSet();

		FAutoVariable_edit = GetGameCoreGlobalAt<Types::FAutoVariable_edit>(F_AUTO_VARIABLE_EDIT_OFFSET);
		GetGameplayDatabase = GetGameCoreGlobalAt<Types::GetGameplayDatabase>(GET_GAMEPLAY_DATABASE_OFFSET);

		Cache::GetInstance = GetGameCoreGlobalAt<Cache::Types::GetInstance>(Cache::GET_INSTANCE_OFFSET);

		Initializers::orig_InitializePlayerRandomAgendas =
			GetGameCoreGlobalAt<Initializers::Types::InitializePlayerRandomAgendas>(Initializers::INITIALIZE_PLAYER_RANDOM_AGENDAS_OFFSET);
		CreateHook(Initializers::orig_InitializePlayerRandomAgendas, &Initializers::InitializePlayerRandomAgendas, &Initializers::base_InitializePlayerRandomAgendas);

		Initializers::orig_NewGamePlayerRandomAgendas =
			GetGameCoreGlobalAt<Initializers::Types::NewGamePlayerRandomAgendas>(Initializers::NEW_GAME_PLAYER_RANDOM_AGENDAS_OFFSET);
		CreateHook(Initializers::orig_NewGamePlayerRandomAgendas, &Initializers::NewGamePlayerRandomAgendas, &Initializers::base_NewGamePlayerRandomAgendas);
	}
}
