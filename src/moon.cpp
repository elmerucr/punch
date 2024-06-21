/*
 * moon.cpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#include "moon.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include "terminal.hpp"

system_t *sys;

static int l_pokeb(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	uint8_t value = lua_tointeger(L, 2);
	sys->core->write8(address, value);
	return 0;
}

static int l_peekb(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	lua_pushinteger(L, sys->core->read8(address));
	return 1;
}

static int l_pokew(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	uint16_t value = lua_tointeger(L, 2);
	sys->core->write8(address, value >> 8);
	sys->core->write8(address + 1, value & 0xff);
	return 0;
}

static int l_peekw(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	lua_pushinteger(L, (sys->core->read8(address) << 8) | sys->core->read8(address + 1));
	return 1;
}

moon_t::moon_t(system_t *s)
{
	system = sys = s;
}

moon_t::~moon_t()
{
	if(L != nullptr) {
		/*
		 * Clean up Lua
		 */
		printf("[Moon] Closing Lua\n");
		lua_close(L);
	}
}

const char lua_init_code[] = R"Lua(

-- callback functions

function frame()
  print('frame')
end

function init()
  print('init')
end

function timer0()
  print('timer0')
end

function timer1()
  print('timer1')
end

function timer2()
  print('timer2')
end

function timer3()
  print('timer3')
end

function timer4()
  print('timer4')
end

function timer5()
  print('timer5')
end

function timer6()
  print('timer6')
end

function timer7()
  print('timer7')
end

print('moon is running init code')

)Lua";

void moon_t::reset()
{
	if(L != nullptr) {
		/*
		 * Clean up Lua
		 */
		printf("[Moon] Closing Lua\n");
		lua_close(L);
		
		L = nullptr;
	}
	
	/*
	 * Start Lua
	 */
	if (!L) L = luaL_newstate();

	if (!L) {
		printf("[Moon] error, couldn't start Lua\n");
		// TODO: failure... exit?
	} else {
		printf("[Moon] %s\n", LUA_COPYRIGHT);
	}
	
	luaL_openlibs(L);
	
	lua_pushcfunction(L, l_pokeb);
	lua_setglobal(L, "pokeb");
	lua_pushcfunction(L, l_peekb);
	lua_setglobal(L, "peekb");
	lua_pushcfunction(L, l_pokew);
	lua_setglobal(L, "pokew");
	lua_pushcfunction(L, l_peekw);
	lua_setglobal(L, "peekw");
	
	/*
	 * Load "resident" Lua program into system
	 */
	if (luaL_dostring(L, lua_init_code)) {
		printf("[Moon] Lua error: %s", lua_tostring(L, -1));
	}
}

uint8_t moon_t::io_read8(uint16_t address)
{
	return 0x00;
}

void moon_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xf) {
		case 0x00:
			// status register
			break;
		case 0x01:
			// control register
			if (value & 0b00000001) {
				lua_getglobal(L, "frame");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b10000000) {
				lua_getglobal(L, "init");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			break;
		case 0x02:
			if (value & 0b00000001) {
				lua_getglobal(L, "timer0");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b00000010) {
				lua_getglobal(L, "timer1");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b00000100) {
				lua_getglobal(L, "timer2");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b00001000) {
				lua_getglobal(L, "timer3");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b00010000) {
				lua_getglobal(L, "timer4");
				 if (lua_pcall(L, 0, 0, 0)) {
					 printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				 }
			}
			if (value & 0b00100000) {
				lua_getglobal(L, "timer5");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b01000000) {
				lua_getglobal(L, "timer6");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			if (value & 0b10000000) {
				lua_getglobal(L, "timer7");
				if (lua_pcall(L, 0, 0, 0)) {
					printf("[Moon] Lua error: %s\n", lua_tostring(L, -1));
				}
			}
			break;
		default:
			break;
	}
}

bool moon_t::load(const char *p)
{
	bool return_value = false;
	
	if (luaL_dofile(L, p)) {
		system->debugger->terminal->printf("\n[Scriptor] Error: %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		system->switch_to_debug_mode();
		return_value = true;
	}
	
	return return_value;
}
