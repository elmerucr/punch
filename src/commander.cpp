/*
 * commander.cpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#include "commander.hpp"
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

static SQInteger s_poke(HSQUIRRELVM v)
{
	SQInteger address;
	SQInteger value;
	sq_getinteger(v, -2, &address);
	sq_getinteger(v, -1, &value);
	sys->core->write8((uint16_t)address, (uint8_t)value);
	return 0;
}

static SQInteger s_peek(HSQUIRRELVM v)
{
	SQInteger address;
	sq_getinteger(v, -1, &address);
	sq_pushinteger(v, sys->core->read8((uint16_t)address));
	return 1;
}

static SQInteger s_poke16(HSQUIRRELVM v)
{
	SQInteger address;
	SQInteger value;
	sq_getinteger(v, -2, &address);
	sq_getinteger(v, -1, &value);
	sys->core->write8((uint16_t)address, (uint16_t)value >> 8);
	sys->core->write8((uint16_t)address + 1, (uint16_t)value & 0xff);
	return 0;
}

static SQInteger s_peek16(HSQUIRRELVM v)
{
	SQInteger address;
	sq_getinteger(v, -1, &address);
	sq_pushinteger(v, (sys->core->read8((uint16_t)address) << 8) | sys->core->read8((uint16_t)address + 1));
	return 1;
}

commander_t::commander_t(system_t *s)
{
	system = sys = s;
}

commander_t::~commander_t()
{
	if (L != nullptr) {
		/*
		 * Clean up Lua
		 */
		printf("[Lua] Closing Lua\n");
		lua_close(L);
	}
	
	if (v != nullptr) {
		/*
		 * Clean up Squirrel
		 */
		system->debugger->terminal->printf("[Squirrel] Closing virtual machine\n");
		sq_close(v);
	}
}

const char lua_init_code[] = R"Lua(

print('[Lua] Running initialization code')

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

const char squirrel_init_code[] = R"Squirrel(

print("\n[Squirrel] Running initialization code")

//function fib(n)
//{
//	if (n < 2) return 1
//	return fib(n - 2) + fib(n - 1)
//}
//
//for (local i = 0; i <= 20; i++) {
//	print("\n" + i + " " + fib(i))
//}

)Squirrel";

void printfunc(HSQUIRRELVM SQ_UNUSED_ARG(v), const SQChar *s, ...)
{
	char buffer[1024];
	va_list arglist;
	va_start(arglist, s);
	//vprintf(s, arglist);
	vsnprintf(buffer, 1024, s, arglist);
	va_end(arglist);
	sys->debugger->terminal->puts(buffer);
}

void errorfunc(HSQUIRRELVM SQ_UNUSED_ARG(v),const SQChar *s,...)
{
	char buffer[1024];
	va_list vl;
	va_start(vl, s);
	//vprintf(s, vl);
	vsnprintf(buffer, 1024, s, vl);
	va_end(vl);
	sys->debugger->terminal->puts(buffer);
}

void commander_t::reset()
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
	
	/*
	 * Squirrel initialization stuff
	 */
	if (v != nullptr) {
		sq_close(v);
		system->debugger->terminal->printf("\n[Squirrel] Closing virtual machine");
		v = nullptr;
	}
	
	v = sq_open(1024);
	sqstd_seterrorhandlers(v);
	sq_setprintfunc(v, printfunc, errorfunc);
	
	if (v) {
		system->debugger->terminal->printf("\n[Squirrel] %s - %s", SQUIRREL_VERSION, SQUIRREL_COPYRIGHT);
	} else {
		system->debugger->terminal->printf("\n[Squirrel] Error: Failed to open virtual machine");
	}
	
	/*
	 * Register global functions, placed well here???? TODO: check
	 */
	
	sq_pushroottable(v);
	sq_pushstring(v, "poke", -1);
	sq_newclosure(v, s_poke, 0);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);
	
	sq_pushroottable(v);
	sq_pushstring(v, "peek", -1);
	sq_newclosure(v, s_peek, 0);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);
	
	sq_pushroottable(v);
	sq_pushstring(v, "poke16", -1);
	sq_newclosure(v, s_poke16, 0);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);
	
	sq_pushroottable(v);
	sq_pushstring(v, "peek16", -1);
	sq_newclosure(v, s_peek16, 0);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);
	
	/*
	 * load resident squirrel code into virtual machine
	 */
	if (sq_compilebuffer(v, squirrel_init_code, strlen(squirrel_init_code), "squirrel_init_code", SQTrue)) {
		system->debugger->terminal->printf("\n[Squirrel] Error sq_compilebuffer");
	} else {
		sq_pushroottable(v);
		
		if (sq_call(v, 1, SQFalse, SQTrue)) {
			system->debugger->terminal->printf("\n[Squirrel] Error");
		}
	}
}

uint8_t commander_t::io_read8(uint16_t address)
{
	return 0x00;
}

void commander_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xff) {
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
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00000010) {
				lua_getglobal(L, "timer1");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00000100) {
				lua_getglobal(L, "timer2");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00001000) {
				lua_getglobal(L, "timer3");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b00010000) {
				lua_getglobal(L, "timer4");
				 if (lua_pcall(L, 0, 0, 0)) {
					 system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					 lua_pop(L, 1);
					 system->switch_to_debug_mode();
				 }
			}
			if (value & 0b00100000) {
				lua_getglobal(L, "timer5");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b01000000) {
				lua_getglobal(L, "timer6");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			if (value & 0b10000000) {
				lua_getglobal(L, "timer7");
				if (lua_pcall(L, 0, 0, 0)) {
					system->debugger->terminal->printf("\n[Lua error] %s", lua_tostring(L, -1));
					lua_pop(L, 1);
					system->switch_to_debug_mode();
				}
			}
			break;
		case 0x80:
			// status register squirrel
			break;
		case 0x81:
			// control register
			if (value & 0b00000001) {
				// frame
			}
			if (value & 0b10000000) {
				// call init
				sq_pushroottable(v);
				sq_pushstring(v,"init",-1);
				sq_get(v,-2); //get the function from the root table
				sq_pushroottable(v); //'this' (function environment object)
				if (sq_call(v,1,SQFalse,SQFalse)) {
					system->debugger->terminal->printf("\n[Squirrel] Error");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			break;
		default:
			break;
	}
}

bool commander_t::load_lua(const char *p)
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

static SQInteger file_lexfeedASCII(SQUserPointer file)
{
	int ret;
	char c;
	if( ( ret=fread(&c,sizeof(c),1,(FILE *)file )>0) )
		return c;
	return 0;
}

bool commander_t::load_squirrel(const char *p)
{
	bool return_value = false;
	
	FILE *f = fopen(p, "rb");
	
	if (f) {
		sq_compile(v,file_lexfeedASCII,f,p,1);
		fclose(f);
		//return_value = false;
	}
	
	sq_pushroottable(v);
	
	if (sq_call(v, 1, SQFalse, SQTrue)) {
		system->debugger->terminal->printf("\n[Squirrel] Error");
		system->switch_to_debug_mode();
	}
	
	return return_value;
}
