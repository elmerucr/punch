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
#include "rca.hpp"

class debugger_t {
public:
	debugger_t(system_t *s);
	~debugger_t();

	system_t *system;

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
	
	rca bruce_rand;
private:
	surface_t font;
	surface_t framebuffer;
	tile_surface_t character_screen;
	surface_t bruce;

	const uint8_t fg = 0x23;
	const uint8_t fg_acc = 0x33;
	const uint8_t bg = 0x00;
	const uint8_t bg_acc = 0x55;

	uint8_t irq_no;

	char text_buffer[2048];

	bool have_prompt{true};

	const uint8_t bruce_data[3*21*8] = {
		0x01,0x01,0x54,0x54,0x01,0x01,0x01,0x01,	// ____bbbb________
		0x01,0x54,0x54,0xfb,0x01,0x01,0x01,0x01,	// __bbbb..________
		0x01,0x54,0xfb,0xfb,0x54,0x01,0x01,0x01,	// __bb....bb______
		0x01,0x54,0xfb,0x54,0xfb,0x01,0x01,0x01,	// __bb..bb..______
		0x01,0x54,0xfb,0xfb,0xfb,0x01,0x01,0x01,	// __bb......______
		0x01,0x01,0xfb,0xfb,0xfb,0x01,0x01,0x01,	// ____......______
		0x01,0x01,0xfb,0xfb,0x01,0x01,0x01,0x01,	// ____....________
		0x01,0xfb,0xfb,0xfb,0x01,0x01,0x01,0x01,	// __......________
		0x01,0xfb,0xfb,0xfb,0xfb,0x01,0x01,0x01,	// __........______
		0x01,0xfb,0xfb,0x54,0xfb,0xfb,0x01,0x54,	// __....bb....__bb
		0x01,0xfb,0x54,0x54,0xfb,0xfb,0xfb,0x54,	// __..bbbb......bb
		0x01,0x01,0xfb,0xfb,0xfb,0x01,0x01,0x01,	// ____......______
		0x01,0x01,0x54,0x54,0x54,0x01,0x01,0x01,	// ____bbbbbb______
		0x01,0x01,0x54,0x54,0x54,0x01,0x01,0x01,	// ____bbbbbb______
		0x01,0x01,0x54,0x01,0x54,0x54,0x01,0x01,	// ____bb__bbbb____
		0x01,0x01,0x54,0x01,0x01,0x54,0x01,0x01,	// ____bb____bb____
		0x01,0x01,0x54,0x01,0x01,0x54,0x01,0x01,	// ____bb____bb____
		0x01,0x01,0x54,0x01,0x01,0x54,0x01,0x01,	// ____bb____bb____
		0x01,0x01,0x54,0x01,0x01,0x54,0x01,0x01,	// ____bb____bb____
		0x01,0x54,0x54,0x01,0x01,0x54,0x54,0x01,	// __bbbb____bbbb__
		0x54,0x54,0x54,0x01,0x01,0x54,0x54,0x54,	// bbbbbb____bbbbbb

		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x54,0x54,0x54,	// __________bbbbbb
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0xfb,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0x54,	// ________bbbb..bb
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0xfb,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x54,0xfb,0xfb,0xfb,	// ________bb......
		0x01,0x01,0x01,0x01,0xfb,0xfb,0xfb,0x01,	// ________......__
		0x01,0x01,0x01,0xfb,0xfb,0xfb,0x01,0x01,	// ______......____
		0x01,0x01,0x01,0xfb,0xfb,0xfb,0x01,0x01,	// ______......____
		0x01,0x01,0x01,0xfb,0xfb,0xfb,0x01,0x01,	// ______......____
		0x01,0x01,0x54,0xfb,0xfb,0xfb,0x01,0x01,	// ____bb......____
		0x01,0x54,0x54,0x54,0xfb,0x54,0x01,0x01,	// __bbbbbb..bb____
		0x01,0x01,0x54,0x54,0x54,0x54,0x01,0x01,	// ____bbbbbbbb____
		0x01,0x01,0x54,0x54,0x54,0x54,0x01,0x01,	// ____bbbbbbbb____
		0x01,0x01,0x01,0x54,0x54,0x01,0x01,0x01,	// ______bbbb______
		0x01,0x01,0x01,0x54,0x54,0x01,0x01,0x01,	// ______bbbb______
		0x01,0x01,0x01,0x54,0x01,0x01,0x01,0x01,	// ______bb________
		0x01,0x01,0x54,0x54,0x01,0x01,0x01,0x01,	// ____bbbb________
		0x01,0x01,0x54,0x01,0x01,0x01,0x01,0x01,	// ____bb__________
		0x01,0x01,0x54,0x01,0x01,0x01,0x01,0x01,	// ____bb__________

		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,	// ________________
		0x01,0x01,0x01,0x01,0x01,0x54,0x54,0x54,	// __________bbbbbb
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0xfb,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0x54,	// ________bbbb..bb
		0x01,0x01,0x01,0x01,0x54,0x54,0xfb,0xfb,	// ________bbbb....
		0x01,0x01,0x01,0x01,0x54,0xfb,0xfb,0xfb,	// ________bb......
		0x01,0x01,0xfb,0xfb,0xfb,0xfb,0xfb,0x01,	// ____..........__
		0x01,0xfb,0xfb,0xfb,0xfb,0xfb,0x01,0x01,	// __..........____
		0x01,0xfb,0x01,0xfb,0xfb,0xfb,0xfb,0x01,	// __..__........__
		0x01,0xfb,0x01,0xfb,0xfb,0xfb,0xfb,0x01,	// __..__........__
		0x01,0x54,0x01,0x54,0xfb,0xfb,0xfb,0x01,	// __bb__bb......__
		0x01,0x54,0x01,0x54,0x54,0x01,0xfb,0x54,	// __bb__bbbb__..bb
		0x01,0x54,0x54,0x54,0x54,0x01,0x01,0x54,	// __bbbbbbbb____bb
		0x01,0x01,0x54,0x54,0x54,0x01,0x01,0x01,	// ____bbbbbb______
		0x01,0x01,0x01,0x54,0x01,0x54,0x01,0x01,	// ______bb__bb____
		0x01,0x01,0x54,0x54,0x01,0x54,0x54,0x01,	// ____bbbb__bbbb__
		0x54,0x54,0x01,0x01,0x01,0x01,0x54,0x01,	// bbbb________bb__
		0x54,0x01,0x01,0x01,0x01,0x01,0x54,0x01,	// bb__________bb__
		0x54,0x01,0x01,0x01,0x01,0x01,0x54,0x01,	// bb__________bb__
		0x01,0x01,0x01,0x01,0x01,0x01,0x54,0x54		// ____________bbbb
	};
};

#endif
