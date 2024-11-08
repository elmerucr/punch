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

static SQInteger s_vpoke(HSQUIRRELVM v)
{
	SQInteger address;
	SQInteger value;
	sq_getinteger(v, -2, &address);
	sq_getinteger(v, -1, &value);
	sys->core->blitter->vram[address & VRAM_SIZE_MASK] = (uint8_t)value;
	return 0;
}

static SQInteger s_vpeek(HSQUIRRELVM v)
{
	SQInteger address;
	sq_getinteger(v, -1, &address);
	sq_pushinteger(v, sys->core->blitter->vram[address & VRAM_SIZE_MASK]);
	return 1;
}

commander_t::commander_t(system_t *s)
{
	system = sys = s;
}

commander_t::~commander_t()
{
	if (v != nullptr) {
		/*
		 * Clean up Squirrel
		 */
		system->debugger->terminal->printf("[Squirrel] Closing virtual machine\n");
		sq_close(v);
	}
}

const char squirrel_init_code[] = R"Squirrel(

print("\n[Squirrel] Running initialization code")

/*
 * Next code to disable too much functionality from iolib, but we want to
 * keep dofile and loadfile...
 */
function writeclosuretofile(destpath, closure) {}
class file {}

function breakhere(number)
{
	for (local i = 0; i < number; i++) {
		::suspend()
	}
}

function pset(x0, y0, color, surface)
{
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x08)
}

function line(x0, y0, x1, y1, color, surface)
{
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x10)
}

function rectangle(x0, y0, x1, y1, color, surface)
{
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x20)
}

function solid_rectangle(x0, y0, x1, y1, color, surface)
{
	poke16(0xe08, x0)
	poke16(0xe0a, y0)
	poke16(0xe0c, x1)
	poke16(0xe0e, y1)
	poke(0xe05, color)
	poke(0xe03, surface)
	poke(0xe01, 0x40)
}

midi <- [
	0x008b, 0x0093, 0x009c, 0x00a6, 0x00af, 0x00ba, 0x00c5, 0x00d1,
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
]

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
	 * Register io lib
	 */
	//sqstd_register_bloblib(v);	// not necessary??? manual states that this one needs to be loaded as well...
	sq_pushroottable(v);
	sqstd_register_iolib(v);

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

	sq_pushroottable(v);
	sq_pushstring(v, "vpoke", -1);
	sq_newclosure(v, s_vpoke, 0);
	sq_newslot(v, -3, SQFalse);
	sq_pop(v, 1);

	sq_pushroottable(v);
	sq_pushstring(v, "vpeek", -1);
	sq_newclosure(v, s_vpeek, 0);
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
	// empty
	return 0x00;
}

void commander_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x1f) {
		case 0x00:
			// status register squirrel
			break;
		case 0x01:
			// control register
			if (value & 0b00000001) {
				// frame
				sq_pushroottable(v);
				sq_pushstring(v, "frame", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling frame() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b10000000) {
				// call init
				sq_pushroottable(v);
				sq_pushstring(v, "init", -1);
				sq_get(v,-2); //get the function from the root table
				sq_pushroottable(v); //'this' (function environment object)
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling init() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			break;
		case 0x02:
			if (value & 0b00000001) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer0", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer0() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b00000010) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer1", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer1() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b00000100) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer2", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer2() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b00001000) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer3", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer3() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b00010000) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer4", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer4() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b00100000) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer5", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer5() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b01000000) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer6", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer6() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			if (value & 0b10000000) {
				sq_pushroottable(v);
				sq_pushstring(v, "timer7", -1);
				sq_get(v, -2);
				sq_pushroottable(v);
				if (sq_call(v,1,SQFalse,SQTrue)) {
					system->debugger->terminal->printf("\n[Squirrel] Error calling timer7() function");
					system->switch_to_debug_mode();
				}
				sq_pop(v,2); //pops the roottable and the function
			}
			break;
		default:
			break;
	}
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
	system->debugger->terminal->printf("\n[Squirrel] Running %s", p);

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
