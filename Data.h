#pragma once
#include "HavokScript.h"
#include <unordered_map>
#include <string>
#include <variant>

namespace Data {
    struct LuaVariant : public std::variant<std::string, double, int> {
        void push(hks::lua_State* L) const;
    };

    struct LuaVariantMap : public std::unordered_map<std::string, LuaVariant> {
        LuaVariantMap() = default;
        void rebuild(hks::lua_State* L);
        void reclaim(hks::lua_State* L, const std::string& propertyToGet);
    };
}