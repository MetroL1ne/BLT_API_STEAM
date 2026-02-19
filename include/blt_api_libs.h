#pragma once

#include <superblt_flat.h>

void LuaState_SetGlobalCallback(lua_State* L, const char* callback_id) {
	if (!lua_isfunction(L, 1)) {
		printf("[SteamCB] ERROR: %d called with non-function (type=%s)\n", callback_id, lua_type(L, 1));
		return;
	}

	// lua_pushvalue(L, 1);
	lua_setglobal(L, callback_id);

	printf("[SteamCB] Ready callback registered to global: %s\n", callback_id);
}

bool LuaState_SafeGetGlobal(lua_State* L, const char *global_id) {
	__try {
		lua_getglobal(L, global_id);
	}
	__except (1) {
		return false;
	}

	return true;
}