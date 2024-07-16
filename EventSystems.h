#pragma once
#include "Data.h"
#include "HavokScript.h"

namespace EventSystems {
	extern void CallCustomProcessor(const char* name, Data::LuaVariantMap& variantMap);
	extern int lRegisterProcessor(hks::lua_State* L);
}