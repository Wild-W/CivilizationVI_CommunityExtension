#include "EventSystems.h"
#include "Runtime.h"
#include <vector>
#include "Game.h"
#include <mutex>
#include <functional>

namespace EventSystems {
	struct ProcessorEvent {
		hks::lua_State* L;
		int callbackIndex;
	};

	std::mutex eventsMutex;
	std::unordered_map<std::string, std::vector<ProcessorEvent>> processorEvents;

	int lRegisterProcessor(hks::lua_State* L) {
		size_t length;
		const char* name = hks::checklstring(L, 1, &length);
		hks::pushvalue(L, 2);
		int callbackIndex = hks::ref(L, hks::LUA_REGISTRYINDEX);

		std::lock_guard<std::mutex> lock(eventsMutex);
		processorEvents[name].push_back(ProcessorEvent{ L, callbackIndex });
		return 0;
	}

	bool DoesProcessorExist(const std::string& name) {
		auto eventIterator = processorEvents.find(name);
		return eventIterator != processorEvents.end();
	}

	static int iteration = 0;
	static void print_stack(hks::lua_State* L) {
		int top = hks::gettop(L);
		for (int i = 1; i <= top; i++) {
			int type = hks::type(L, i);
			switch (type) {
			case 4:
				size_t _;
				printf(" String: '%s'\n", hks::checklstring(L, i, &_));
				break;
			case 2:
				printf(" Boolean: %s\n", hks::toboolean(L, i) ? "true" : "false");
				break;
			case 3:
				printf(" Number: %g\n", hks::tonumber(L, i));
				break;
			default:
				printf(" Other %d\n", type);
			}
		}
	}

	static bool callProcessorInState(hks::lua_State* L, int callbackIndex, Data::LuaVariantMap& variantMap, const std::string& propertyToGet = "") {
		hks::rawgeti(L, hks::LUA_REGISTRYINDEX, callbackIndex);

		// Push the variant map as a Lua table onto the stack
		hks::createtable(L, 0, variantMap.size());
		for (const auto& pair : variantMap) {
			hks::pushfstring(L, pair.first.c_str());
			pair.second.push(L);
			hks::settable(L, -3);
		}

		int tableIndex = hks::ref(L, hks::LUA_REGISTRYINDEX);
		hks::rawgeti(L, hks::LUA_REGISTRYINDEX, tableIndex);

		if (hks::pcall(L, 1, 1, 0) != 0) {
			size_t length;
			std::cout << "Error calling processor: " << hks::checklstring(L, -1, &length) << "!\n";
			hks::unref(L, hks::LUA_REGISTRYINDEX, tableIndex); // Clean up reference in case of error
			return false;
		}

		bool result = hks::toboolean(L, -1);
		hks::rawgeti(L, hks::LUA_REGISTRYINDEX, tableIndex);
		if (propertyToGet.empty()) {
			variantMap.rebuild(L);
		}
		else {
			variantMap.reclaim(L, propertyToGet);
		}

		hks::pop(L, 2);
		hks::unref(L, hks::LUA_REGISTRYINDEX, tableIndex);

		return result;
	}

	bool CallProcessors(const std::string& name, Data::LuaVariantMap& variantMap, const std::string& propertyToGet) {
		auto eventIterator = processorEvents.find(name);
		if (eventIterator == processorEvents.end()) {
			return false;
		}
		
		for (const auto& processor : eventIterator->second) {
			if (callProcessorInState(processor.L, processor.callbackIndex, variantMap, propertyToGet)) {
				return true;
			}
		}
		return false;
	}
}

namespace Firaxis::EventSystems {
	void Create() {
		using namespace Runtime;


	}
}