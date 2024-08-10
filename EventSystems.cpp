#include "EventSystems.h"
#include "Runtime.h"
#include <vector>
#include "Game.h"
#include <mutex>
#include <functional>

namespace EventSystems {
	struct ProcessorEvent {
		std::function<bool(Data::LuaVariantMap& variantMap, const std::string& propertyToGet)> callback;
	};

	std::mutex eventsMutex;
	std::unordered_map<std::string, std::vector<ProcessorEvent>> logicEvents;

	int lRegisterProcessor(hks::lua_State* L) {
		size_t length;
		const char* name = hks::checklstring(L, 1, &length);
		hks::pushvalue(L, 2);
		int callbackIndex = hks::ref(L, hks::LUA_REGISTRYINDEX);

		std::lock_guard<std::mutex> lock(eventsMutex);
		logicEvents[name].push_back(ProcessorEvent{
			[L, callbackIndex](Data::LuaVariantMap& variantMap, const std::string& propertyToGet) {
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
			});
		return 0;
	}

	bool DoesProcessorExist(const std::string& name) {
		auto eventIterator = logicEvents.find(name);
		return eventIterator != logicEvents.end();
	}

	bool CallCustomProcessor(const std::string& name, Data::LuaVariantMap& variantMap, const std::string& propertyToGet) {
		auto eventIterator = logicEvents.find(name);
		if (eventIterator == logicEvents.end()) {
			std::cout << "Could not find processor: " << name << '\n';
			return false;
		}
		
		for (const auto& processor : eventIterator->second) {
			if (processor.callback(variantMap, propertyToGet)) {
				return true;
			}
		}
		return false;
	}
}