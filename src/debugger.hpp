/*
 * debugger.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef DEBUGGER_HPP
#define DEBUGGER_HPP

#include "system.hpp"
#include "blitter.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "terminal.hpp"

class debugger_t {
public:
	debugger_t(system_t *s);
	~debugger_t();
	
	system_t *system;
	
	void redraw();
	
	void run(int32_t *cd); // cd = cycles done
	
	blitter_ic *blitter;
	
	terminal_t *terminal;
	
	int32_t process_command(char *c);
	void prompt();
	void print_version();
	void status();
	
	void memory_dump(uint16_t address);
	void enter_memory_line(char *buffer);
	
	bool hex_string_to_int(const char *temp_string, uint32_t *return_value);
private:
	surface_t framebuffer;
	tile_surface_t character_screen;
	surface_t bruce;
	
	const uint8_t fg = 0b00110100;
	const uint8_t fg_acc = 0b11100000;
	const uint8_t bg = 0b00000000;
	const uint8_t bg_acc = 0b00000100;
	
	uint8_t irq_no;
	
	char text_buffer[2048];
	
	bool have_prompt{true};
	
	const uint8_t bruce_data[3*21*8] = {
		0x01,0x01,0x00,0x00,0x01,0x01,0x01,0x01,
		0x01,0x00,0x00,0xda,0x01,0x01,0x01,0x01,
		0x01,0x00,0xda,0xda,0x00,0x01,0x01,0x01,
		0x01,0x00,0xda,0x00,0xda,0x01,0x01,0x01,
		0x01,0x00,0xda,0xda,0xda,0x01,0x01,0x01,
		0x01,0x01,0xda,0xda,0xda,0x01,0x01,0x01,
		0x01,0x01,0xda,0xda,0x01,0x01,0x01,0x01,
		0x01,0xda,0xda,0xda,0x01,0x01,0x01,0x01,
		0x01,0xda,0xda,0xda,0xda,0x01,0x01,0x01,
		0x01,0xda,0xda,0x00,0xda,0xda,0x01,0x00,
		0x01,0xda,0x00,0x00,0xda,0xda,0xda,0x00,
		0x01,0x01,0xda,0xda,0xda,0x01,0x01,0x01,
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,
		0x01,0x01,0x00,0x01,0x00,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x00,0x00,0x01,0x01,0x00,0x00,0x01,
		0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,
		
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,	// __________bbbbbb
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0xda,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0x00,	// ________bbbb..bb
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0xda,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x00,0xda,0xda,0xda,	// ________bb......
		0x01,0x01,0x01,0x01,0xda,0xda,0xda,0x01,	// ________......__
		0x01,0x01,0x01,0xda,0xda,0xda,0x01,0x01,	// ______......____
		0x01,0x01,0x01,0xda,0xda,0xda,0x01,0x01,	// ______......____
		0x01,0x01,0x01,0xda,0xda,0xda,0x01,0x01,	// ______......____
		0x01,0x01,0x00,0xda,0xda,0xda,0x01,0x01,	// ____bb......____
		0x01,0x00,0x00,0x00,0xda,0x00,0x01,0x01,	// __bbbbbb..bb____
		0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,	// ____bbbbbbbb____
		0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,	// ____bbbbbbbb____
		0x01,0x01,0x01,0x00,0x00,0x01,0x01,0x01,	// ______bbbb______
		0x01,0x01,0x01,0x00,0x00,0x01,0x01,0x01,	// ______bbbb______
		0x01,0x01,0x01,0x00,0x01,0x01,0x01,0x01,	// ______bb________
		0x01,0x01,0x00,0x00,0x01,0x01,0x01,0x01,	// ____bbbb________
		0x01,0x01,0x00,0x01,0x01,0x01,0x01,0x01,	// ____bb__________
		0x01,0x01,0x00,0x01,0x01,0x01,0x01,0x01,	// ____bb__________
		
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,	// __________bbbbbb
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0xda,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0x00,	// ________bbbb..bb
		0x01,0x01,0x01,0x01,0x00,0x00,0xda,0xda,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x00,0xda,0xda,0xda,	// ________bb......
		0x01,0x01,0xda,0xda,0xda,0xda,0xda,0x01,	// ____..........__
		0x01,0xda,0xda,0xda,0xda,0xda,0x01,0x01,	// __..........____
		0x01,0xda,0x01,0xda,0xda,0xda,0xda,0x01,	// __..__........__
		0x01,0xda,0x01,0xda,0xda,0xda,0xda,0x01,	// __..__........__
		0x01,0x00,0x01,0x00,0xda,0xda,0xda,0x01,	// __bb__bb......__
		0x01,0x00,0x01,0x00,0x00,0x01,0xda,0x00,	// __bb__bbbb__..bb
		0x01,0x00,0x00,0x00,0x00,0x01,0x01,0x00,	// __bbbbbbbb____bb
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,	// ____bbbbbb______
		0x01,0x01,0x01,0x00,0x01,0x00,0x01,0x01,	// ______bb__bb____
		0x01,0x01,0x00,0x00,0x01,0x00,0x00,0x01,	// ____bbbb__bbbb__
		0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x01,	// bbbb________bb__
		0x00,0x01,0x01,0x01,0x01,0x01,0x00,0x01,	// bb__________bb__
		0x00,0x01,0x01,0x01,0x01,0x01,0x00,0x01,	// bb__________bb__
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00		// ____________bbbb
	};
};

#endif

// 0b11011010
