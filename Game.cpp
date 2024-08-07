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
			variantMap.emplace("VisibilityType", LuaVariant(visibilityType));
			variantMap.emplace("ExcludePlayerId", LuaVariant(playerId));

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
}

namespace Game {
	namespace Cache {
		Cache::Types::GetInstance GetInstance;
	}
	Types::FAutoVariable_edit FAutoVariable_edit;

	void Create() {
		using namespace Runtime;

		FAutoVariable_edit = GetGameCoreGlobalAt<Types::FAutoVariable_edit>(F_AUTO_VARIABLE_EDIT_OFFSET);
		GetGameplayDatabase = GetGameCoreGlobalAt<Types::GetGameplayDatabase>(GET_GAMEPLAY_DATABASE_OFFSET);

		Cache::GetInstance = GetGameCoreGlobalAt<Cache::Types::GetInstance>(Cache::GET_INSTANCE_OFFSET);

		Initializers::orig_InitializePlayerRandomAgendas =
			GetGameCoreGlobalAt<Initializers::Types::InitializePlayerRandomAgendas>(Initializers::INITIALIZE_PLAYER_RANDOM_AGENDAS_OFFSET);
		CreateHook(Initializers::orig_InitializePlayerRandomAgendas, &Initializers::InitializePlayerRandomAgendas, &Initializers::base_InitializePlayerRandomAgendas);
	}
}
