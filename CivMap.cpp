#include "CivMap.h"
#include "Runtime.h"
#include "EventSystems.h"
#include "HavokScript.h"

namespace CivMap::Improvement::Builder {
	Types::CanHaveDistrict orig_CanHaveDistrict;
	Types::CanHaveDistrict base_CanHaveDistrict;
	bool CanHaveDistrict(
		void* builder, Plot::Instance* plot, int districtIndex,
		int playerId, bool b1, bool b2, bool b3, bool b4, City::Instance* city
	)
	{
		using namespace EventSystems;
		using namespace Data;

		if (DoesProcessorExist("CanHaveDistrict")) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("DistrictIndex", LuaVariant(districtIndex));
			variantMap.emplace("CityId", LuaVariant(*(int*)((uintptr_t)city + 0xa8)));
			variantMap.emplace("PlayerId", LuaVariant(playerId));
			variantMap.emplace("X", LuaVariant(*(short*)((uintptr_t)plot + 0x28)));
			variantMap.emplace("Y", LuaVariant(*(short*)((uintptr_t)plot + 0x2a)));
			variantMap.emplace("b1", LuaVariant(b1));
			variantMap.emplace("b2", LuaVariant(b2));
			variantMap.emplace("b3", LuaVariant(b3));
			variantMap.emplace("b4", LuaVariant(b4));
			
			variantMap.emplace("Result", LuaVariant(-1));

			if (CallCustomProcessor("CanHaveDistrict", variantMap, "Result")) {
				int result = std::get<int>(variantMap.at("Result"));

				if (result == 0) return false;
				else if (result == 1) return true;
			}
		}

		return base_CanHaveDistrict(builder, plot, districtIndex, playerId, b1, b2, b3, b4, city);
	}

	static void Create() {
		using namespace Runtime;

		orig_CanHaveDistrict = GetGameCoreGlobalAt<Types::CanHaveDistrict>(CAN_HAVE_DISTRICT_OFFSET);
		CreateHook(orig_CanHaveDistrict, &CanHaveDistrict, &base_CanHaveDistrict);
	}
}


namespace CivMap {
	void Create() {
		using namespace Runtime;

		Improvement::Builder::Create();
	}
}