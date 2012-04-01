#include "../luawrap.hpp"
#include "l_sprite.h"
#include "l_level_player.h"
#include "../../level/level_player.h"
using namespace SMC;

/***************************************
 * Event handlers
 ***************************************/

LUA_IMPLEMENT_EVENT(jump);
LUA_IMPLEMENT_EVENT(shoot);

/***************************************
 * "Normal" acces
 ***************************************/

/**
 * downgrade()
 *
 * Hurt the player. Kills him if he is small.
 */
static int Lua_Level_Player_Downgrade(lua_State* p_state)
{
	if (!lua_isuserdata(p_state, 1))
		return luaL_error(p_state, "No receiver (userdata) given.");

	pLevel_Player->DownGrade();
	return 0;
}

static int Lua_Level_Player_Jump(lua_State* p_state)
{
	if (!lua_isuserdata(p_state, 1))
		return luaL_error(p_state, "No receiver (userdata) given.");

	if (lua_isnumber(p_state, 2)){
		float deaccel = static_cast<float>(lua_tonumber(p_state, 2));
		pLevel_Player->Start_Jump(deaccel);
	}
	else
		pLevel_Player->Start_Jump();

	return 0;
}

/**
 * set_type( type )
 *
 * Apply a powerup to the player (or shrink him). The possible
 * types you can pass are the following strings:
 *
 * "dead":  Please use the kill() method instead.
 * "small": Please use the downgrade() method instead.
 * "big":   Apply the normal mushroom.
 * "fire":  Apply the fireplant.
 * "ice"    Apply the ice mushroom.
 * "ghost": Apply the ghost mushroom.
 */
static int Lua_Level_Player_Set_Type(lua_State* p_state)
{
	if (!lua_isuserdata(p_state, 1))
		return luaL_error(p_state, "No receiver (userdata) given.");

	std::string type_str = std::string(luaL_checkstring(p_state, 2));
	Maryo_type type;

	if (type_str == "dead")
		type = MARYO_DEAD;
	else if (type_str == "small")
		type = MARYO_SMALL;
	else if (type_str == "big")
		type = MARYO_BIG;
	else if (type_str == "fire")
		type = MARYO_FIRE;
	else if (type_str == "ice")
		type = MARYO_ICE;
	//else if (type_str == "cape") // Not implemented officially by SMC
	//	type = MARY_CAPE;
	else if (type_str == "ghost")
		type = MARYO_GHOST;
	else
		return luaL_error(p_state, "Invalid type '%s'.", type_str.c_str());

	pLevel_Player->Set_Type(type);

	return 0;
}

/**
 * Kill()
 *
 * Immediately kill the player.
 */
static int Lua_Level_Player_Kill(lua_State* p_state)
{
	if (!lua_isuserdata(p_state, 1))
		return luaL_error(p_state, "No receiver (userdata) given.");

	pLevel_Player->DownGrade(true);
	return 0;
}

static luaL_Reg Player_Methods[] = {
	{"downgrade", Lua_Level_Player_Downgrade},
	{"jump", Lua_Level_Player_Jump},
	{"kill", Lua_Level_Player_Kill},
	{"on_jump", LUA_EVENT_HANDLER(jump)},
	{"on_shoot", LUA_EVENT_HANDLER(shoot)},
	{"set_type", Lua_Level_Player_Set_Type},
	{NULL, NULL}
};

void Script::Open_Level_Player(lua_State* p_state)
{
	LuaWrap::register_subclass<cLevel_Player>(p_state,
																						"LevelPlayer",
																						"AnimatedSprite",
																						Player_Methods,
																						NULL,
																						NULL, // Singleton, cannot allocate new one
																						NULL); // Memory managed by SMC, no finalizer needed

	// Make the global variable Player point to the sole instance
	// of the LevelPlayer class.
	lua_getglobal(p_state, "LevelPlayer"); // Class table needed for instanciation further below
	cLevel_Player** pp_player = (cLevel_Player**) lua_newuserdata(p_state, sizeof(cLevel_Player*));
	*pp_player = pLevel_Player;
	LuaWrap::InternalC::set_imethod_table(p_state); // Attach instance methods
	// Cleanup the stack, remove the class table
	lua_insert(p_state, -2);
	lua_pop(p_state, 1);
	// Make the global variable
	lua_setglobal(p_state, "Player");
}
