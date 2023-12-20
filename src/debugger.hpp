#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "app.hpp"
#include "blitter.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "terminal.hpp"

class debugger_t {
public:
	debugger_t(app_t *a);
	~debugger_t();
	
	app_t *app;
	
	void redraw();
	
	void run();
	
	blitter_ic *blitter;
	
	terminal_t *terminal;
	
	void process_command(char *c);
	void prompt();
	void print_version();
	void status();
	
	void memory_dump(uint16_t address);
	void enter_memory_line(char *buffer);
	
	bool hex_string_to_int(const char *temp_string, uint32_t *return_value);
private:
	tile_surface character_screen;
	
	uint8_t irq_no;
	
//	core_t *core;
//	keyboard_t *keyboard;
	
	char text_buffer[2048];
	
	bool have_prompt{true};
};

#endif
