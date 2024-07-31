#pragma once
#include "HavokScript.h"

namespace NationalParks {
	typedef class NationalParks;
	namespace Types {
		typedef NationalParks* (*__cdecl Edit)(void);
		typedef void (*__thiscall DesignatePark)(NationalParks*, int playerId, int* plots);
	}

	constexpr uintptr_t GET_NATIONAL_PARKS_OFFSET = 0x2363e0;
	extern Types::Edit Edit;

	constexpr uintptr_t DESIGNATE_PARK_OFFSET = 0x235830;
	extern Types::DesignatePark DesignatePark;

	extern int Register(hks::lua_State* L);
	extern void Create();
}