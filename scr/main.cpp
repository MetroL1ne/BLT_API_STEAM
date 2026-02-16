#define _CRT_SECURE_NO_WARNINGS

#include <blt_api_libs.h>
#include <steam/steam_api.h>

#include <stdlib.h>

#ifndef LUA_NOREF
#define LUA_NOREF (-2)
#endif

#ifndef LUA_REFNIL
#define LUA_REFNIL (-1)
#endif

#ifndef LUA_OK
#define LUA_OK (0)
#endif

static lua_State* g_L = nullptr;

/*

// Steam Init
int BLT_API_STEAM_SteamAPI_Init(lua_State* L){
	SteamAPI_Init();

	return 0;
}

// Steam Shutdown
int BLT_API_STEAM_SteamAPI_Shutdown(lua_State* L){
	SteamAPI_Shutdown();

	return 0;
}

// ============================== Steam.SteamUser ==============================

// Get local player steam level
int BLT_API_STEAM_GetPlayerSteamLevel(lua_State* L){
	CSteamID steamID = SteamUser()->GetSteamID();
	int32 level = SteamUser()->GetPlayerSteamLevel();

	lua_pushinteger(L, level);

	return 1;
}

// ============================== Steam.SteamNetworking ==============================

// Send P2P Packet
int BLT_API_STEAM_SteamNetworking_SendP2PPacket(lua_State* L){
	CSteamID steam_id;
	uint64_t steam_id64 = strtoull(luaL_checkstring(L, 1), nullptr, 10);
	steam_id.SetFromUint64(steam_id64);

	size_t data_len = 0;
	const char* pubData = luaL_checklstring(L, 2, &data_len);
	uint32 cubData = (uint32)data_len;

	EP2PSend eP2PSendType = (EP2PSend)luaL_checkinteger(L, 3);
	int nChannel = luaL_checkinteger(L, 4);

	// Send
	bool result = SteamNetworking()->SendP2PPacket(
		steam_id,
		pubData,
		cubData,
		eP2PSendType,
		nChannel
	);
	
	// Return to Lua
	lua_pushboolean(L, result);

	return 1;
}

*/

// ============================== Steam.SteamFriends ==============================

// --------- Steam.SteamFriends Hooks ---------


class BLT_API_STEAM_SteamFriends_Callbacks
{
private:
	// On Game Overlay Activated (Steam default keybind "Shift + Tab" Menu)
	STEAM_CALLBACK(BLT_API_STEAM_SteamFriends_Callbacks, OnGameOverlayActivated, GameOverlayActivated_t);
};

// On Game Overlay Activated (Steam default keybind "Shift + Tab" Menu)
void BLT_API_STEAM_SteamFriends_Callbacks::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
{
	if (pCallback->m_bActive)
		printf("[Steam] Overlay now active\n");
	else
		printf("[Steam] Overlay now inactive\n");
}

// ============================== Steam.SteamInventory ==============================

int BLT_API_STEAM_SteamInventory_SetSteamInventoryRequestPricesResultCallback(lua_State* L) {
	BLT_API_SetCallback(L, "SteamInventoryRequestPricesResultCallback");
	g_L = L;

	return 0;
}

int BLT_API_STEAM_SteamInventory_GetItemPrice(lua_State* L) {
	// Item ID
	SteamItemDef_t iDefinition = luaL_checkinteger(L, 1);
	// Current Price
	uint64 pPrice = 0;
	// Base Price
	uint64 bPrice = 0;

	bool result = SteamInventory()->GetItemPrice(iDefinition, &pPrice, &bPrice);
	double current_price = pPrice / 100;
	double base_price = bPrice / 100;

	lua_pushboolean(L, result);
	lua_pushnumber(L, current_price);
	lua_pushnumber(L, base_price);

	return 3;
}

// --------- Steam.SteamInventory Hooks ---------

class BLT_API_STEAM_SteamInventory_Callbacks
{
public:
	void RequestPrices();

private:
	void OnSteamInventoryRequestPricesResult(SteamInventoryRequestPricesResult_t *pCallback, bool bIOFailure);
	CCallResult< BLT_API_STEAM_SteamInventory_Callbacks, SteamInventoryRequestPricesResult_t > m_SteamInventoryRequestPricesResult;
};

// 进行异步请求，以接收当前玩家数量。
void BLT_API_STEAM_SteamInventory_Callbacks::RequestPrices()
{
	SteamAPICall_t hSteamAPICall = SteamInventory()->RequestPrices();
	m_SteamInventoryRequestPricesResult.Set(hSteamAPICall, this, &BLT_API_STEAM_SteamInventory_Callbacks::OnSteamInventoryRequestPricesResult);

	printf("Requested Prices\n");
}

// 调用 SteamAPI_RunCallbacks() 后，在 .SteamUserStats()->GetNumberOfCurrentPlayers() 异步返回时调用。
void BLT_API_STEAM_SteamInventory_Callbacks::OnSteamInventoryRequestPricesResult(SteamInventoryRequestPricesResult_t *pCallback, bool bIOFailure)
{
	if (pCallback->m_result)
		printf("Request Inventory Prices Succeeful\n");
	else
		printf("Request Inventory Prices Failed\n");

	lua_getglobal(g_L, "SteamInventoryRequestPricesResultCallback");

	// Param 1：bool Result
	lua_pushboolean(g_L, pCallback->m_result == k_EResultOK);
	// Param 2：string Local Currency
	lua_pushstring(g_L, pCallback->m_rgchCurrency);

	// protected call
	if (lua_pcall(g_L, 2, 0, 0) != LUA_OK) {
		const char* err = lua_tostring(g_L, -1);
		printf("[Steam] SteamInventoryRequestPricesResult callback error: %s\n", err);
		lua_pop(g_L, 1);
	}
}

// Call Hook Function
BLT_API_STEAM_SteamInventory_Callbacks g_SteamInventoryCallbacks;
int BLT_API_STEAM_SteamInventory_RequestPrices(lua_State* L) {
	g_SteamInventoryCallbacks.RequestPrices();

	return 0;
}













// ============================== Steam.SteamScreenshots ==============================

// Trigger Screenshot
int BLT_API_STEAM_SteamScreenshots_TriggerScreenshot(lua_State* L) {
	SteamScreenshots()->TriggerScreenshot();

	return 0;
}

// SteamScreenshots_SetLocation
int BLT_API_STEAM_SteamScreenshots_SetLocation(lua_State* L) {
	uint32 m_hLocal = (uint32)luaL_checkint(L, 1);
	const char* locaction_name = luaL_checkstring(L, 2);

	SteamScreenshots()->SetLocation(m_hLocal, locaction_name);

	return 0;
}

// SteamScreenshots_TagUser
int BLT_API_STEAM_SteamScreenshots_TagUser(lua_State* L) {
	uint32 m_hLocal = (uint32)luaL_checkint(L, 1);

	// String steam id to CSteamID
	CSteamID steam_id;
	steam_id.SetFromUint64(strtoull(luaL_checkstring(L, 2), nullptr, 10));

	SteamScreenshots()->TagUser(m_hLocal, steam_id);

	return 0;
}

int BLT_API_STEAM_SteamScreenshots_SetScreenshotReadyCallback(lua_State* L) {
	BLT_API_SetCallback(L, "ScreenshotReadyCallback");
	g_L = L;

	return 0;
}

// --------- Steam.SteamScreenshots Hooks ---------

class BLT_API_STEAM_SteamScreenshots_Callbacks
{
private:
	// On
	STEAM_CALLBACK(BLT_API_STEAM_SteamScreenshots_Callbacks, OnScreenshotReady, ScreenshotReady_t);
};

// On Screenshot Ready (Steam default keybind "F12")
void BLT_API_STEAM_SteamScreenshots_Callbacks::OnScreenshotReady(ScreenshotReady_t* pCallback)
{
	if (pCallback->m_eResult == k_EResultOK)
		printf("Screenshot Write Succeeful\n");
	else {
		printf("Screenshot Write Failed\n");
	}

	lua_getglobal(g_L, "ScreenshotReadyCallback");

	// Param 1：uint32 ScreenshotHandle
	lua_pushinteger(g_L, pCallback->m_hLocal);
	// Param 2：bool EResult
	lua_pushboolean(g_L, pCallback->m_eResult == k_EResultOK);
		
	// protected call
	if (lua_pcall(g_L, 2, 0, 0) != LUA_OK) {
		const char* err = lua_tostring(g_L, -1);
		printf("[Steam] ScreenshotReady callback error: %s\n", err);
		lua_pop(g_L, 1);
	}
}

// ================================================================================

void SetCallback() {
	static BLT_API_STEAM_SteamFriends_Callbacks SteamFriends_Callbacks;
	static BLT_API_STEAM_SteamInventory_Callbacks SteamInventory_Callbacks;
	static BLT_API_STEAM_SteamScreenshots_Callbacks SteamScreenshots_Callbacks;
}

// ================================================================================


/* 以下是SuperBLT API内容 */

void Plugin_Init(){
	printf("BLT_API_STEAM loaded successfully\n");

	SetCallback();
}

void Plugin_Update(){
	SteamAPI_RunCallbacks();
}

void Plugin_Setup_Lua(lua_State* L){
}

int Plugin_PushLua(lua_State* L){
	// Steam
	lua_newtable(L);

	/*
	lua_pushcfunction(L, BLT_API_STEAM_SteamAPI_Shutdown);
	lua_setfield(L, -2, "SteamAPI_Shutdown");

	// Steam.SteamUser
	lua_newtable(L);

	lua_pushcfunction(L, BLT_API_STEAM_GetPlayerSteamLevel);
	lua_setfield(L, -2, "GetPlayerSteamLevel");

	lua_setfield(L, -2, "SteamUser");

	// Steam.SteamNetworking
	lua_newtable(L);

	lua_pushcfunction(L, BLT_API_STEAM_SteamNetworking_SendP2PPacket);
	lua_setfield(L, -2, "SendP2PPacket");

	lua_setfield(L, -2, "SteamNetworking");
	*/

	// Steam.SteamInventory
	lua_newtable(L);

	lua_pushcfunction(L, BLT_API_STEAM_SteamInventory_RequestPrices);
	lua_setfield(L, -2, "RequestPrices");

	lua_pushcfunction(L, BLT_API_STEAM_SteamInventory_SetSteamInventoryRequestPricesResultCallback);
	lua_setfield(L, -2, "SetSteamInventoryRequestPricesResultCallback");

	lua_pushcfunction(L, BLT_API_STEAM_SteamInventory_GetItemPrice);
	lua_setfield(L, -2, "GetItemPrice");

	lua_setfield(L, -2, "SteamInventory");

	// Steam.SteamScreenshots
	lua_newtable(L);

	lua_pushcfunction(L, BLT_API_STEAM_SteamScreenshots_TriggerScreenshot);
	lua_setfield(L, -2, "TriggerScreenshot");

	lua_pushcfunction(L, BLT_API_STEAM_SteamScreenshots_SetLocation);
	lua_setfield(L, -2, "SetLocation");

	lua_pushcfunction(L, BLT_API_STEAM_SteamScreenshots_TagUser);
	lua_setfield(L, -2, "TagUser");

	lua_pushcfunction(L, BLT_API_STEAM_SteamScreenshots_SetScreenshotReadyCallback);
	lua_setfield(L, -2, "SetScreenshotReadyCallback");

	lua_setfield(L, -2, "SteamScreenshots");

	return 1;
}