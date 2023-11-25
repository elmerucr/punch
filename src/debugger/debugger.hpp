#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "blitter.hpp"

class debugger_t {
public:
	debugger_t();
	~debugger_t();
	
	void redraw();
	
	blitter_ic *blitter;
private:
	surface screen;
	tile_surface terminal;
};

#endif
