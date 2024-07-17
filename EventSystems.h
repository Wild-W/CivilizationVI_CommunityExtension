#pragma once
#include "Data.h"
#include "HavokScript.h"

namespace EventSystems {
	extern bool DoesProcessorExist(const char* name);
	extern bool CallCustomProcessor(const char* name, Data::LuaVariantMap& variantMap);
	extern int lRegisterProcessor(hks::lua_State* L);
}