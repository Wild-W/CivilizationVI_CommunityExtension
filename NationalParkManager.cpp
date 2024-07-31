#include "NationalParks.h"
#include "Runtime.h"

namespace NationalParks {
	Types::DesignatePark DesignatePark;
	Types::Edit Edit;

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

	int Register(hks::lua_State* L) {
		std::cout << "Registering NationalParks!\n";
		hks::createtable(L, 0, 1);

		PushLuaMethod(L, lDesignatePark, "lDesignatePark", -2, "DesignatePark");

		hks::pushlightuserdata(L, Edit());
		hks::setfield(L, -2, "__instance");

		hks::setfield(L, hks::LUA_GLOBAL, "NationalParks");
		return 0;
	}

	void Create() {
		using namespace Runtime;

		DesignatePark = GetGameCoreGlobalAt<Types::DesignatePark>(DESIGNATE_PARK_OFFSET);
		Edit = GetGameCoreGlobalAt<Types::Edit>(GET_NATIONAL_PARKS_OFFSET);
	}
}