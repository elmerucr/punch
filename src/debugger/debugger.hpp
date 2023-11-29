#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "blitter.hpp"
#include "core.hpp"
#include "terminal.hpp"

class debugger_t {
public:
	debugger_t(core_t *c);
	~debugger_t();
	
	void redraw();
	
	blitter_ic *blitter;
	
	terminal_t *terminal;
private:
	surface font_surface;
	tile_surface character_screen;
	
	core_t *core;
};

#endif
