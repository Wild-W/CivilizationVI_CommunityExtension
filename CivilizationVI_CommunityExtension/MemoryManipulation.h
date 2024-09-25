#pragma once
#include "HavokScript.h"
#include "MinHook.h"
#include "Runtime.h"

namespace MemoryManipulation {
    enum FieldType {
        FIELD_BYTE,
        FIELD_SHORT,
        FIELD_UNSIGNED_SHORT,
        FIELD_INT,
        FIELD_UNSIGNED_INT,
        FIELD_LONG_LONG,
        FIELD_UNSIGNED_LONG_LONG,
        FIELD_CHAR,
        FIELD_FLOAT,
        FIELD_DOUBLE,
        FIELD_C_STRING,
        FIELD_BOOL
    };

    namespace LuaExport {
        extern int lMem(hks::lua_State* L);
        extern int lObjMem(hks::lua_State* L);
        extern int lRegisterCallEvent(hks::lua_State* L);

        extern void PushFieldTypes(hks::lua_State* L);
    }
}