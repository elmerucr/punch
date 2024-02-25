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
	void enter_assembly_line(char *buffer);
	
	uint32_t disassemble_instruction(uint16_t address);
	
	void vram_dump(uint32_t address, uint32_t width);
	void vram_binary_dump(uint32_t address, uint32_t width);

	bool hex_string_to_int(const char *temp_string, uint32_t *return_value);

	rca bruce_rand;
private:
	/*
	 * color theme
	 */
	const uint8_t fg = 0x23;
	const uint8_t fg_acc = 0x33;
	const uint8_t bg = 0x00;
	const uint8_t bg_acc = 0x55;

	uint8_t irq_no;

	char text_buffer[2048];

	bool have_prompt{true};

	const uint8_t bruce_data[2*21*3] = {
		0b00000101, 0b00000000,	// ____bbbb________
		0b00010110, 0b00000000,	// __bbbb..________
		0b00011010, 0b01000000,	// __bb....bb______
		0b00011001, 0b10000000,	// __bb..bb..______
		0b00011010, 0b10000000,	// __bb......______
		0b00001010, 0b10000000,	// ____......______
		0b00001010, 0b00000000,	// ____....________
		0b00101010, 0b00000000,	// __......________
		0b00101010, 0b10000000,	// __........______
		0b00101001, 0b10100001,	// __....bb....__bb
		0b00100101, 0b10101001,	// __..bbbb......bb
		0b00001010, 0b10000000,	// ____......______
		0b00000101,	0b01000000,	// ____bbbbbb______
		0b00000101, 0b01000000,	// ____bbbbbb______
		0b00000100, 0b01010000,	// ____bb__bbbb____
		0b00000100, 0b00010000,	// ____bb____bb____
		0b00000100, 0b00010000,	// ____bb____bb____
		0b00000100, 0b00010000,	// ____bb____bb____
		0b00000100, 0b00010000,	// ____bb____bb____
		0b00010100, 0b00010100,	// __bbbb____bbbb__
		0b01010100, 0b00010101,	// bbbbbb____bbbbbb
		
		0b00000000, 0b00000000,	// ________________
		0b00000000, 0b00000000,	// ________________
		0b00000000, 0b00010101,	// __________bbbbbb
		0b00000000, 0b01011010,	// ________bbbb....
		0b00000000, 0b01011001,	// ________bbbb..bb
		0b00000000, 0b01011010,	// ________bbbb....
		0b00000000, 0b01101010,	// ________bb......
		0b00000000, 0b10101000,	// ________......__
		0b00000010, 0b10100000,	// ______......____
		0b00000010, 0b10100000,	// ______......____
		0b00000010, 0b10100000,	// ______......____
		0b00000110, 0b10100000,	// ____bb......____
		0b00010101, 0b10010000,	// __bbbbbb..bb____
		0b00000101, 0b01010000,	// ____bbbbbbbb____
		0b00000101, 0b01010000,	// ____bbbbbbbb____
		0b00000001, 0b01000000,	// ______bbbb______
		0b00000001, 0b01000000,	// ______bbbb______
		0b00000001, 0b00000000,	// ______bb________
		0b00000101, 0b00000000,	// ____bbbb________
		0b00000100, 0b00000000,	// ____bb__________
		0b00000100, 0b00000000,	// ____bb__________

		0b00000000, 0b00000000,	// ________________
		0b00000000,	0b00000000,	// ________________
		0b00000000,	0b00010101,	// __________bbbbbb
		0b00000000, 0b01011010,	// ________bbbb....
		0b00000000, 0b01011001,	// ________bbbb..bb
		0b00000000, 0b01011010,	// ________bbbb....
		0b00000000, 0b01101010,	// ________bb......
		0b00001010, 0b10101000,	// ____..........__
		0b00101010, 0b10100000,	// __..........____
		0b00100010, 0b10101000,	// __..__........__
		0b00100010, 0b10101000,	// __..__........__
		0b00010001, 0b10101000,	// __bb__bb......__
		0b00010001, 0b01001001,	// __bb__bbbb__..bb
		0b00010101, 0b01000001,	// __bbbbbbbb____bb
		0b00000101, 0b01000000,	// ____bbbbbb______
		0b00000001, 0b00010000,	// ______bb__bb____
		0b00000101, 0b00010100,	// ____bbbb__bbbb__
		0b01010000, 0b00000100,	// bbbb________bb__
		0b01000000, 0b00000100,	// bb__________bb__
		0b01000000, 0b00000100,	// bb__________bb__
		0b00000000, 0b00000101	// ____________bbbb
	};
};

#endif
