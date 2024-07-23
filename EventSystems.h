#pragma once
#include "Data.h"
#include "HavokScript.h"
#include <string>

namespace EventSystems {
	extern bool DoesProcessorExist(const std::string& name);
	extern bool CallCustomProcessor(const std::string& name, Data::LuaVariantMap& variantMap);
	extern int lRegisterProcessor(hks::lua_State* L);
}