#include "NationalParks.h"
#include "Runtime.h"

namespace NationalParks {
	Types::DesignatePark DesignatePark;
	Types::Edit Edit;
	Types::FindPark FindPark;
	Types::RestoreVisualState RestoreVisualState;

	namespace Cache {
		Cache::Types::Update Update;
	}

	//bool DesignateParkInPlots(NationalParks* nationalParks, int playerId, int* baseCoord, const std::vector<int>& plotIds) {
	//	DesignatePark(nationalParks, playerId, baseCoord);
	//	ParkData* park = FindPark(nationalParks, baseCoord);
	//
	//	if (park == NULL) {
	//		return false;
	//	}
	//
	//
	//
	//	return true;
	//}

	//static int lDesignateParkInPlots(hks::lua_State* L) {
	//	int playerId = hks::checkinteger(L, 1);
	//	int coord[2]{
	//		hks::checkinteger(L, 2),
	//		hks::checkinteger(L, 3)
	//	};
	//
	//	int arraySize = hks::objlen(L, 2);
	//	std::vector<int> plotIds;
	//	for (int i = 1; i <= arraySize; i++) {
	//		hks::pushinteger(L, i);
	//		hks::gettable(L, 2);
	//
	//		int plotId = hks::checkinteger(L, -1);
	//
	//		plotIds.push_back(plotId);
	//		hks::pop(L, 1);
	//	}
	//
	//	hks::pushboolean(L, DesignateParkInPlots(Edit(), playerId, coord, plotIds));
	//	return 1;
	//}

	static int lDesignatePark(hks::lua_State* L) {
		int playerId = hks::checkinteger(L, 1);
		// x, y coordinate
		int coord[2] {
			hks::checkinteger(L, 2),
			hks::checkinteger(L, 3)
		};

		DesignatePark(Edit(), playerId, coord);
		return 0;
	}

	static int lFindPark(hks::lua_State* L) {
		int coord[2]{
			hks::checkinteger(L, 1),
			hks::checkinteger(L, 2)
		};

		ParkData* park = FindPark(Edit(), coord);
		if (park != NULL) {
			hks::pushlightuserdata(L, park);
			return 1;
		}
		return 0;
	}

	static int lRestoreVisualState(hks::lua_State* L) {
		if (!hks::isuserdata(L, 1)) {
			hks::error(L, "Type mismatch: Expected userdata pointer to ParkData object!\n");
			return 0;
		}
		auto park = (ParkData*)hks::touserdata(L, 1);

		hks::pushboolean(L, RestoreVisualState(Edit(), park));
		return 1;
	}

	int Register(hks::lua_State* L) {
		std::cout << "Registering NationalParks!\n";
		hks::createtable(L, 0, 3);

		PushLuaMethod(L, lDesignatePark, "lDesignatePark", -2, "DesignatePark");
		PushLuaMethod(L, lFindPark, "lFindPark", -2, "FindPark");
		PushLuaMethod(L, lRestoreVisualState, "lRestoreVisualState", -2, "RestoreVisualState");

		hks::pushlightuserdata(L, Edit());
		hks::setfield(L, -2, "__instance");

		hks::setfield(L, hks::LUA_GLOBAL, "NationalParks");
		return 0;
	}

	void Create() {
		using namespace Runtime;

		DesignatePark = GetGameCoreGlobalAt<Types::DesignatePark>(DESIGNATE_PARK_OFFSET);
		Edit = GetGameCoreGlobalAt<Types::Edit>(GET_NATIONAL_PARKS_OFFSET);
		RestoreVisualState = GetGameCoreGlobalAt<Types::RestoreVisualState>(RESTORE_VISUAL_STATE_OFFSET);
		FindPark = GetGameCoreGlobalAt<Types::FindPark>(FIND_PARK_OFFSET);
	}
}