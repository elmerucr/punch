/*
 * luna.cpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#include "luna.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include "terminal.hpp"

system_t *sys;

static int l_poke(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	uint8_t value = lua_tointeger(L, 2);
	sys->core->write8(address, value);
	return 0;
}

static int l_peek(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	lua_pushinteger(L, sys->core->read8(address));
	return 1;
}

static int l_poke16(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	uint16_t value = lua_tointeger(L, 2);
	sys->core->write8(address, value >> 8);
	sys->core->write8(address + 1, value & 0xff);
	return 0;
}

static int l_peek16(lua_State *L)
{
	uint16_t address = lua_tointeger(L, 1);
	lua_pushinteger(L, (sys->core->read8(address) << 8) | sys->core->read8(address + 1));
	return 1;
}

luna_t::luna_t(system_t *s)
{
	system = sys = s;
}

luna_t::~luna_t()
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

print('[Lua] Running init code')

-- callback functions

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

function breakhere(no)
	for i=1, no do
		coroutine.yield()
	end
end

midi = {
	[0] = 0x008b, 0x0093, 0x009c, 0x00a6, 0x00af, 0x00ba, 0x00c5, 0x00d1,
	0x00dd, 0x00ea, 0x00f8, 0x0107, 0x0116, 0x0127, 0x0139, 0x014b,
	0x015f, 0x0174, 0x018a, 0x01a1, 0x01ba, 0x01d4, 0x01f0, 0x020e,
	0x022d, 0x024e, 0x0271, 0x0296, 0x02be, 0x02e7, 0x0314, 0x0342,
	0x0374, 0x03a9, 0x03e0, 0x041b, 0x045a, 0x049c, 0x04e2, 0x052d,
	0x057b, 0x05cf, 0x0627, 0x0685, 0x06e8, 0x0751, 0x07c1, 0x0837,
	0x08b4, 0x0938, 0x09c4, 0x0a59, 0x0af7, 0x0b9d, 0x0c4e, 0x0d0a,
	0x0dd0, 0x0ea2, 0x0f81, 0x106d, 0x1167, 0x1270, 0x1389, 0x14b2,
	0x15ed, 0x173b, 0x189c, 0x1a13, 0x1ba0, 0x1d45, 0x1f02, 0x20da,
	0x22ce, 0x24e0, 0x2711, 0x2964, 0x2bda, 0x2e76, 0x3139, 0x3426,
	0x3740, 0x3a89, 0x3e04, 0x41b4, 0x459c, 0x49c0, 0x4e23, 0x52c8,
	0x57b4, 0x5ceb, 0x6272, 0x684c, 0x6e80, 0x7512, 0x7c08, 0x8368,
	0x8b39, 0x9380, 0x9c45, 0xa590, 0xaf68, 0xb9d6, 0xc4e3, 0xd099,
	0xdd00, 0xea24, 0xf810
}

function line(x0, y0, x1, y1, color, surface)
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x08)
end

function rectangle(x0, y0, x1, y1, color, surface)
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x10)
end

function solid_rectangle(x0, y0, x1, y1, color, surface)
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x20)
end

)Lua";

void luna_t::reset()
{
	if(L != nullptr) {
		/*
		 * Clean up Lua
		 */
		printf("[Lua] Closing Lua\n");
		lua_close(L);
		
		L = nullptr;
	}
	
	/*
	 * Start Lua
	 */
	if (!L) L = luaL_newstate();

	if (!L) {
		printf("[Lua error] couldn't start Lua\n");
		// TODO: failure... exit?
	} else {
		printf("[Lua] %s\n", LUA_COPYRIGHT);
	}
	
	luaL_openlibs(L);
	
	lua_pushcfunction(L, l_poke);
	lua_setglobal(L, "poke");
	lua_pushcfunction(L, l_peek);
	lua_setglobal(L, "peek");
	lua_pushcfunction(L, l_poke16);
	lua_setglobal(L, "poke16");
	lua_pushcfunction(L, l_peek16);
	lua_setglobal(L, "peek16");
	
	/*
	 * Load "resident" Lua program into system
	 */
	if (luaL_dostring(L, lua_init_code)) {
		printf("[Lua] Error: %s\n", lua_tostring(L, -1));
	}
}

uint8_t luna_t::io_read8(uint16_t address)
{
	return 0x00;
}

void luna_t::io_write8(uint16_t address, uint8_t value)
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
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b10000000) {
				lua_getglobal(L, "init");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			break;
		case 0x02:
			if (value & 0b00000001) {
				lua_getglobal(L, "timer0");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00000010) {
				lua_getglobal(L, "timer1");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00000100) {
				lua_getglobal(L, "timer2");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00001000) {
				lua_getglobal(L, "timer3");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00010000) {
				lua_getglobal(L, "timer4");
				 if (lua_pcall(L, 0, 0, 0)) {
					 system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					 lua_pop(L, 1);
					 system->switch_to_debug_mode();
				 }
			}
			if (value & 0b00100000) {
				lua_getglobal(L, "timer5");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b01000000) {
				lua_getglobal(L, "timer6");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b10000000) {
				lua_getglobal(L, "timer7");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("[Lua error] %s\n", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			break;
		default:
			break;
	}
}

bool luna_t::load(const char *p)
{
	bool return_value = false;
	
	if (luaL_dofile(L, p)) {
		system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
		lua_pop(L, 1);
		system->switch_to_debug_mode();
		return_value = true;
	}
	
	return return_value;
}
