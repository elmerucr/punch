#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "blitter.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "terminal.hpp"

class debugger_t {
public:
	debugger_t(core_t *c, keyboard_t *k);
	~debugger_t();
	
	void redraw();
	
	void run();
	
	blitter_ic *blitter;
	
	terminal_t *terminal;
	
	void process_command(char *c);
	void prompt();
	void print_version();
	void status();
private:
	tile_surface character_screen;
	
	core_t *core;
	keyboard_t *keyboard;
	
	char text_buffer[2048];
};

#endif
