#include "AI.h"
#include "Runtime.h"
#include "EventSystems.h"
#include "Data.h"

namespace AI {
	namespace CongressSupport {
		Types::CongressSupport::District orig_District;
		Types::CongressSupport::District base_District;

		bool District(Types::CongressSupport::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
			using namespace EventSystems;
			using namespace Data;

			std::cout << "Creating variant map!\n";
			auto variantMap = LuaVariantMap();
			variantMap.emplace("OutcomeType", LuaVariant(outcomeType));
			variantMap.emplace("DistrictType", LuaVariant(*(int*)((uintptr_t)modifierAnalysis + 0x1c))); // District Index
			variantMap.emplace("PlayerId", LuaVariant(*(int*)(uintptr_t)player + 0xd8)); // Player Id

			std::cout << "Calling DistrictTargetChooser!\n";
			CallCustomProcessor("DistrictTargetChooser", variantMap);

			std::cout << std::get<int>(variantMap.at("OutcomeType")) << '\n';
			std::cout << std::get<int>(variantMap.at("DistrictType")) << '\n';
			std::cout << std::get<int>(variantMap.at("PlayerId")) << '\n';

			return base_District(congressSupport, player, outcomeType, modifierAnalysis);
		}

		void Create() {
			using namespace Runtime;

			orig_District = GetGameCoreGlobalAt<Types::CongressSupport::District>(DISTRICT_OFFSET);
			CreateHook(orig_District, &District, &base_District);
		}
	};
}