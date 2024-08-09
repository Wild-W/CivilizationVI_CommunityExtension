#pragma once
#include "Plot.h"
#include "City.h"

namespace CivMap::Improvement::Builder {
	namespace Types {
		typedef bool(__thiscall* CanHaveDistrict)(
			void* builder, Plot::Instance* plot, int districtIndex,
			int playerId, bool b1, bool b2, bool b3, bool b4, City::Instance* city
			);
	}

	constexpr uintptr_t CAN_HAVE_DISTRICT_OFFSET = 0x25a360;
	extern bool CanHaveDistrict(
		void* builder, Plot::Instance* plot, int districtIndex,
		int playerId, bool b1, bool b2, bool b3, bool b4, City::Instance* city
	);
}

namespace CivMap {
	extern void Create();
}