#pragma once
#include "Data.h"
#include "HavokScript.h"
#include <string>
#include <future>

namespace EventSystems {
	namespace Types {
		typedef void* (* PublishEvents)(void*);
	}

	constexpr uintptr_t PUBLISH_EVENTS_OFFSET = 0x6e260;

	extern bool DoesProcessorExist(const std::string& name);
	extern std::future<bool> CallProcessorsAsync(const std::string& name, Data::LuaVariantMap& variantMap, const std::string& propertyToGet = "");
	extern int lRegisterProcessor(hks::lua_State* L);
	extern void PublishEvents(void* _);

	extern void Create();
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
}