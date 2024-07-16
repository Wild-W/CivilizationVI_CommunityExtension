#include "EventSystems.h"
#include "Runtime.h"
#include <vector>

namespace EventSystems {
	std::unordered_map<std::string, std::vector<std::pair<hks::lua_State*, int>>> logicEvents = {};

	int lRegisterProcessor(hks::lua_State* L) {
		size_t length;
		const char* name = hks::checklstring(L, 1, &length);
		std::cout << "Name of logic event: " << name << '\n';

		hks::pushvalue(L, 2);
		int callbackIndex = hks::ref(L, hks::LUA_REGISTRYINDEX);
		std::cout << "Logic function index from lua: " << callbackIndex << '\n';
        
		logicEvents[name].push_back(std::make_pair(L, callbackIndex));
        return 0;
	}

	void CallCustomProcessor(const char* name, Data::LuaVariantMap& variantMap) {
		auto eventIterator = logicEvents.find(name);
		if (eventIterator == logicEvents.end()) {
			std::cout << "Could not find logic event: " << name << '\n';
			return;
		}
		
		for (const auto& luaPair : eventIterator->second) {
			hks::lua_State* L = luaPair.first;
			int callbackIndex = luaPair.second;
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

			if (hks::pcall(L, 1, 0, 0) != 0) {
				size_t length;
				std::cout << "Error calling logic event: " << hks::checklstring(L, -1, &length) << '\n';
				hks::pop(L, 1);
				hks::unref(L, hks::LUA_REGISTRYINDEX, tableIndex); // Clean up reference in case of error
				return;
			}

			hks::rawgeti(L, hks::LUA_REGISTRYINDEX, tableIndex);
			variantMap.rebuild(L);

			hks::pop(L, 1);
			hks::unref(L, hks::LUA_REGISTRYINDEX, tableIndex);
		}
	}
}