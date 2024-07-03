#pragma once
#include "HavokScript.h"
#include "City.h"

namespace CityTradeManager {
	typedef class ICityTrade;
	namespace Types {
		typedef void(__thiscall* SetHasConstructedTradingPost)(City::Trade* trade, int playerId, bool didConstruct);
		typedef void* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
	}

	constexpr uintptr_t SET_HAS_CONSTRUCTED_TRADING_POST_OFFSET = 0x14e720;

	extern Types::SetHasConstructedTradingPost SetHasConstructedTradingPost;

    extern int lSetHasConstructedTradingPost(hks::lua_State* L);

	extern int Register(hks::lua_State* L);

	extern void Create();
}
