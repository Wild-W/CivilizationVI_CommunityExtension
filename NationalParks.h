#pragma once
#include "HavokScript.h"
#include "vector"

namespace NationalParks {
	typedef class NationalParks;
	typedef struct ParkData;
	namespace Types {
		typedef NationalParks* (*__cdecl Edit)(void);
		typedef void (*__thiscall DesignatePark)(NationalParks*, int playerId, int* coord);
		typedef bool (*__thiscall RestoreVisualState)(NationalParks*, ParkData*);
		typedef ParkData* (*__thiscall FindPark)(NationalParks*, int* coord);
	}

// #pragma pack(push, 1)
// 	struct ParkData {
// 		int id;
// 		int i1;
// 		eastl::vector* plotIds;
// 		void* p1;
// 		char padding[0x18];
// 		char* parkName;
// 	};
// #pragma pack(pop)

	//extern bool DesignateParkInPlots(NationalParks*, int playerId, int* baseCoord, std::vector<int>& plotIds);

	constexpr uintptr_t GET_NATIONAL_PARKS_OFFSET = 0x2363e0;
	extern Types::Edit Edit;

	constexpr uintptr_t DESIGNATE_PARK_OFFSET = 0x235830;
	extern Types::DesignatePark DesignatePark;

	constexpr uintptr_t RESTORE_VISUAL_STATE_OFFSET = 0x2389f0;
	extern Types::RestoreVisualState RestoreVisualState;

	constexpr uintptr_t FIND_PARK_OFFSET = 0x2362c0;
	extern Types::FindPark FindPark;

	extern int Register(hks::lua_State* L);
	extern void Create();
}