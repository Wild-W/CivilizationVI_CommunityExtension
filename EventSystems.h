#pragma once
#include "Data.h"
#include "HavokScript.h"
#include <string>

namespace EventSystems {
	extern bool DoesProcessorExist(const std::string& name);
	extern bool CallProcessors(const std::string& name, Data::LuaVariantMap& variantMap, const std::string& propertyToGet = "");
	extern int lRegisterProcessor(hks::lua_State* L);
}

// Unused
namespace Firaxis::EventSystems {
	typedef interface IScriptSystem1 IScriptSystem1;
	typedef interface IScriptSystemArgs1 IScriptSystemArgs1;
	namespace Types {
		typedef bool(__cdecl* CallProcessor)(IScriptSystem1* scriptSystem, char* gameEvent, IScriptSystemArgs1* args, Firaxis::Data::TypedVariantMap* variantMap);
	}

	namespace Args {
		namespace Types {
			typedef void (*Handle)(IScriptSystemArgs1*);
		}

		constexpr uintptr_t HANDLE_OFFSET = 0;
		extern Types::Handle Handle;
	}
	
	extern IScriptSystem1* ms_pkScriptSystem;

	constexpr uintptr_t CALL_PROCESSOR_OFFSET = 0;
	extern Types::CallProcessor CallProcessor;

	extern void Create();
}