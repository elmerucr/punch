/*
 * debugger.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "debugger.hpp"
#include "common.hpp"
#include <cstdio>

/*
 * hex2int
 * take a hex string and convert it to a 32bit number (max 8 hex digits)
 * from https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
 *
 * This function is slightly adopted to check for true values. It returns false
 * when there's wrong input.
 */
bool debugger_t::hex_string_to_int(const char *temp_string, uint32_t *return_value)
{
	uint32_t val = 0;
	while (*temp_string) {
		/* Get current character then increment */
		uint8_t byte = *temp_string++;
		/* Transform hex character to the 4bit equivalent number */
		if (byte >= '0' && byte <= '9') {
			byte = byte - '0';
		} else if (byte >= 'a' && byte <='f') {
			byte = byte - 'a' + 10;
		} else if (byte >= 'A' && byte <='F') {
			byte = byte - 'A' + 10;
		} else {
			/* Problem, return false and do not write the return value */
			return false;
		}
		/* Shift 4 to make space for new digit, and add the 4 bits of the new digit */
		val = (val << 4) | (byte & 0xf);
	}
	*return_value = val;
	return true;
}

debugger_t::debugger_t(system_t *s)
{
	system = s;

	irq_no = system->core->exceptions->connect_device("debugger");

	blitter = new blitter_ic();
	blitter->reset();

	/* font surface in slot 0xe */
	blitter->surface[0xe].w = 4;
	blitter->surface[0xe].h = 6;
	blitter->surface[0xe].flags_0 = 0b00'00'00'11;	// use background and foreground color, and 1 bit / pixel
	blitter->surface[0xe].flags_1 = 0b0'0'0'0'00'00;
	blitter->surface[0xe].flags_2 = 0b00000'001;	// select tiny font 4x6

	/* character screen in slot 0xd */
	blitter->surface[0xd].w = 80;
	blitter->surface[0xd].h = 30;
	blitter->surface[0xd].base_address = 0x00010000;
	blitter->surface[0xd].x = 0;
	blitter->surface[0xd].y = 10;
	blitter->surface[0xd].flags_0 = 0b00'00'00'00;

	terminal = new terminal_t(system, &blitter->surface[0xd], blitter, fg, bg);
	terminal->clear();
	print_version();
	terminal->putchar('\n');

	/*
	 * Setting up Bruce Lee
	 */
	blitter->surface[0xc].index = 2;
	blitter->surface[0xc].base_address = 0x000300;
	blitter->surface[0xc].flags_0 = 0b00010000;
	blitter->surface[0xc].flags_1 = 0b00000001;
	blitter->surface[0xc].flags_2 = 0b00000000;
	blitter->surface[0xc].w = 8;
	blitter->surface[0xc].h = 21;
	blitter->surface[0xc].x = 30;
	blitter->surface[0xc].y = 169;
	blitter->surface[0xc].color_table[0b00] = 0xc7;
	blitter->surface[0xc].color_table[0b01] = 0x54;
	blitter->surface[0xc].color_table[0b10] = 0xfb;

	for (int i=0; i<(2*21*3); i++) {
		blitter->vram[blitter->surface[0xc].base_address + i] = bruce_data[i];
	}
}

void debugger_t::print_version()
{
	const uint8_t symbol[19] = {
		ASCII_LF,
		0x00, 0x08, 0x0c, 0x00, ASCII_LF,
		0x08, 0x01, 0x00, 0x09, ASCII_LF,
		0x02, 0x04, 0x00, 0x06, ASCII_LF,
		0x00, 0x02, 0x03
	};
	for (int i=0; i<19; i++) {
		if (symbol[i] == ASCII_LF) {
			terminal->putchar(ASCII_LF);
		} else {
			terminal->putsymbol(symbol[i]);
		}
	}
	terminal->printf("\npunch v%i.%i.%i (C)2023-%i elmerucr",
	       PUNCH_MAJOR_VERSION,
	       PUNCH_MINOR_VERSION,
	       PUNCH_BUILD, PUNCH_YEAR);
}

debugger_t::~debugger_t()
{
	delete terminal;
	delete blitter;
}

void debugger_t::redraw()
{
	blitter->set_pixel_saldo(MAX_PIXELS_PER_FRAME);
	//blitter->clear_surface(PUNCH_BLACK, 0x0);	// no need, everything is redrawn already
	blitter->tile_blit(0xe, 0x0, 0xd);
	blitter->io_write8(0x05, fg);	// set drawing color
	blitter->solid_rectangle(0, 0, 319, 9, 0x0);
	blitter->solid_rectangle(0, 190, 319, 199, 0x0);

	// Bruce Lee
	static int state = 0;
	static int wait = 100;
	static bool right = true;
	static bool change_direction = true;

	if (bruce_visible) {
		right ? blitter->surface[0xc].flags_1 &= 0b11101111 : blitter->surface[0xc].flags_1 |= 0b00010000;

		if (wait < 200) {
			blitter->surface[0xc].index = 0;
			state = 0;
		} else {
			if (change_direction) {
				if (bruce_rand.byte() < 128) right = true; else right = false;
				change_direction = false;
			}

			if (state > 4) {
				blitter->surface[0xc].index = 1;
			} else {
				blitter->surface[0xc].index = 2;
			}

			blitter->surface[0xc].x += 2 * (right ? 1 : -1);
			if (blitter->surface[0xc].x > 340) {
				blitter->surface[0xc].x = -20;
			} else if (blitter->surface[0xc].x < -20) {
				blitter->surface[0xc].x = 340;
			}

			state++; if (state == 8) state = 0;
		}
		wait++; if (wait > 300) {
			wait = 0;
			change_direction = true;
		}
		blitter->blit(0xc, 0x0);
	}
	// end Bruce Lee
}

void debugger_t::run()
{
	uint8_t symbol = 0;

	terminal->process_cursor_state();

	while (system->keyboard->events_waiting()) {
		terminal->deactivate_cursor();
		symbol = system->keyboard->pop_event();
		switch (symbol) {
			case ASCII_F1:
				system->core->run(true);
				status();
				prompt();
				break;
			case ASCII_F2:
				status();
				prompt();
				break;
//			case ASCII_F3:
//				terminal->deactivate_cursor();
//				terminal->printf("run");
//				system->switch_to_run_mode();
//				system->host->events_wait_until_key_released(SDLK_F3);
//				break;
			case ASCII_CURSOR_LEFT:
				terminal->cursor_left();
				break;
			case ASCII_CURSOR_RIGHT:
				terminal->cursor_right();
				break;
			case ASCII_CURSOR_UP:
				terminal->cursor_up();
				break;
			case ASCII_CURSOR_DOWN:
				terminal->cursor_down();
				break;
			case ASCII_BACKSPACE:
				terminal->backspace();
				break;
			case ASCII_LF:
				// TODO: supply textbuffer here to isolate command?
				char command_buffer[256];
				terminal->get_command(command_buffer, 256);
				process_command(command_buffer);
				break;
			default:
				terminal->putchar(symbol);
				break;
		}
		terminal->activate_cursor();
	}
}

void debugger_t::process_command(char *c)
{
	int cnt = 0;
	while ((*c == ' ') || (*c == '.')) {
		c++;
		cnt++;
	}

	have_prompt = true;

	char *token0, *token1, *token2;
	token0 = strtok(c, " ");

	if (token0 == NULL) {
		//have_prompt = false;
	} else if (token0[0] == ':') {
		have_prompt = false;
		enter_memory_line(c);
	} else if (token0[0] == ';') {
		have_prompt = false;
		enter_vram_line(c);
	} else if (token0[0] == '\'') {
		have_prompt = false;
		enter_vram_binary_line(c);
	} else if (token0[0] == ',') {
		have_prompt = false;
		enter_assembly_line(c);
	} else if (strcmp(token0, "b") == 0) {
		bool breakpoints_present = false;
		token1 = strtok(NULL, " ");
		if (token1 == NULL) {
			terminal->printf("\nbreakpoints:");
			for (int i=0; i<65536; i++) {
				if (system->core->cpu->breakpoint_array[i]) {
					breakpoints_present = true;
					terminal->printf("\n$%04x", i);
				}
			}
			if (!breakpoints_present) terminal->printf("\nnone");
		} else {
			uint32_t address;
			if (!hex_string_to_int(token1, &address)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
			} else {
				address &= 0xffff;
				system->core->cpu->toggle_breakpoint(address);
				if (system->core->cpu->breakpoint_array[address]) {
					terminal->printf("\nbreakpoint set at $%04x", address);
				} else {
					terminal->printf("\nbreakpoint removed at $%04x", address);
				}
			}
		}
	} else if (strcmp(token0, "bruce") == 0) {
		bruce_visible = !bruce_visible;
	} else if (strcmp(token0, "cls") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "d") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc = system->core->cpu->get_pc();

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->printf("\n.");
				temp_pc += disassemble_instruction(temp_pc);
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->printf("\n.");
					temp_pc += disassemble_instruction(temp_pc);
				}
			}
		}
	} else if (strcmp(token0, "x") == 0) {
		terminal->printf("\nexit punch (y/n)");
		redraw();
		//blitter->update_framebuffer();
		system->host->update_debugger_texture((uint32_t *)&blitter->vram[FRAMEBUFFER_ADDRESS]);
		system->host->update_screen();
		if (system->host->events_yes_no()) {
			system->running = false;
			system->host->events_wait_until_key_released(SDLK_y);
			have_prompt = false;
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "irq") == 0) {
		system->core->exceptions->toggle(irq_no);
		status();
	} else if (strcmp(token0, "m") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t temp_pc = system->core->cpu->get_pc();

		if (token1 == NULL) {
			for (int i=0; i<lines_remaining; i++) {
				terminal->putchar('\n');
				memory_dump(temp_pc);
				temp_pc = (temp_pc + 8) & 0xffff;
			}
		} else {
			if (!hex_string_to_int(token1, &temp_pc)) {
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				for (int i=0; i<lines_remaining; i++) {
					terminal->putchar('\n');
					memory_dump(temp_pc & 0xffff);
					temp_pc = (temp_pc + 8) & 0xffff;
				}
			}
		}
	} else if (strcmp(token0, "mv") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");
		token2 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t address{0};
		uint32_t width;

		if (token1 == NULL) {
			// no address
			terminal->printf("\nerror: missing arguments");
			have_prompt = true;
		} else {
			// decode address
			if (!hex_string_to_int(token1, &address)) {
				// address is wrong
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				// address is ok
				if (token2 == NULL) {
					// missing argument
					terminal->printf("\nerror: missing arguments");
					have_prompt = true;
				} else {
					// decode width
					if (!hex_string_to_int(token2, &width)) {
						// arg wrong
						terminal->printf("\nerror: '%s' is not a hex number", token2);
						have_prompt = true;
					} else {
						address &= VRAM_SIZE_MASK;
						if (width == 0) width = 1;
						if (width > 0x10) width = 0x10;
						for (int i=0; i<lines_remaining; i++) {
							terminal->putchar('\n');
							vram_dump(address & VRAM_SIZE_MASK, width);
							address = (address + width) & VRAM_SIZE_MASK;
						}
					}
				}
			}
		}
	} else if (strcmp(token0, "mvb") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");
		token2 = strtok(NULL, " ");

		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;

		uint32_t address{0};
		uint32_t width;

		if (token1 == NULL) {
			// no address
			terminal->printf("\nerror: missing arguments");
			have_prompt = true;
		} else {
			// decode address
			if (!hex_string_to_int(token1, &address)) {
				// address is wrong
				terminal->printf("\nerror: '%s' is not a hex number", token1);
				have_prompt = true;
			} else {
				// address is ok
				if (token2 == NULL) {
					// missing argument
					terminal->printf("\nerror: missing arguments");
					have_prompt = true;
				} else {
					// decode width
					if (!hex_string_to_int(token2, &width)) {
						// arg wrong
						terminal->printf("\nerror: '%s' is not a hex number", token2);
						have_prompt = true;
					} else {
						address &= VRAM_SIZE_MASK;
						if (width == 0) width = 1;
						if (width > 0x8) width = 0x8;
						for (int i=0; i<lines_remaining; i++) {
							terminal->putchar('\n');
							vram_binary_dump(address & VRAM_SIZE_MASK, width);
							address = (address + width) & VRAM_SIZE_MASK;
						}
					}
				}
			}
		}
	} else if (strcmp(token0, "n") == 0) {
		system->core->run(true);
		status();
	} else if (strcmp(token0, "pal") == 0) {
		terminal->printf("\n    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
		for (int i=0; i<256; i++) {
			if ((i % 16) == 0) {
				terminal->bg_color = bg;
				terminal->printf("\n ");
				if (((i & 0xf0) >> 4) < 10) {
					terminal->putsymbol(0x30 + ((i & 0xf0) >> 4));
				} else {
					terminal->putsymbol(0x57 + ((i & 0xf0) >> 4));
				}
				terminal->putchar(' ');
			}
			terminal->bg_color = i;
			terminal->printf("   ");
		}
		terminal->bg_color = bg;
	} else if (strcmp(token0, "reset") == 0) {
		terminal->printf("\nreset punch (y/n)");
		redraw();
		//blitter->update_framebuffer();
		system->host->update_debugger_texture((uint32_t *)&blitter->vram[FRAMEBUFFER_ADDRESS]);
		system->host->update_screen();
		if (system->host->events_yes_no()) {
			system->core->reset();
			system->host->events_wait_until_key_released(SDLK_y);
		} else {
			system->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "run") == 0) {
		have_prompt = false;
		system->switch_to_run_mode();
		system->host->events_wait_until_key_released(SDLK_RETURN);
	} else if (strcmp(token0, "s") == 0) {
		status();
	} else if (strcmp(token0, "timer") == 0) {
		for (int i=0; i<8; i++) {
			system->core->timer->status(text_buffer, i);
			terminal->printf("%s", text_buffer);
		}
	} else if (strcmp(token0, "v") == 0) {
		system->host->video_toggle_debugger_viewer();
	} else if (strcmp(token0, "ver") == 0) {
		print_version();
	} else {
		terminal->printf("\r");
		for (int i=0; i<strlen(token0) + cnt; i++) terminal->cursor_right();
		terminal->printf("?");
	}

	if (have_prompt) prompt();
}

void debugger_t::prompt()
{
	terminal->printf("\n.");
}

void debugger_t::status()
{
	terminal->clear();
	terminal->printf("_cpu___________________________________________________________");
	system->core->cpu->status(text_buffer, 2048);
	terminal->printf("\n%s", text_buffer);
	terminal->printf("\n\n_disassembly_________________________");
	uint16_t pc = system->core->cpu->get_pc();
	for (int i=0; i<8; i++) {
		terminal->putchar('\n');
		pc += disassemble_instruction(pc);
	}

	terminal->printf("\n\n_usp___  _ssp___  t_____s___bpm______cycles  IRQ_s__Name_____");

	for (int i=0; i<8; i++) {
		uint16_t usp = (system->core->cpu->get_us() + i) & 0xffff;
		uint8_t usp_b = system->core->read8(usp);
		uint16_t ssp = (system->core->cpu->get_sp() + i) & 0xffff;
		uint8_t ssp_b = system->core->read8(ssp);

		system->core->exceptions->status(text_buffer, 2048, i);

		terminal->printf("\n%04x %02x  %04x %02x  %u %s %s %5u  %10u   %s",
				 usp,
				 usp_b,
				 ssp,
				 ssp_b,
				 i,
				 system->core->timer->io_read_byte(0x01) & (1 << i) ? " on" : "off",
				 system->core->timer->io_read_byte(0x00) & (1 << i) ? "*" : "-",
				 system->core->timer->get_timer_bpm(i),
				 system->core->timer->get_timer_clock_interval(i) - system->core->timer->get_timer_counter(i),
				 text_buffer);
	}

	terminal->printf("\n\n%6i of %6i frame cpu cycles done", system->core->get_cpu_cycle_saldo(), CPU_CYCLES_PER_FRAME);
	terminal->printf("\n%6u of %6u blitter pixel writes left for this frame", system->core->blitter->get_pixel_saldo(), MAX_PIXELS_PER_FRAME);
}

void debugger_t::memory_dump(uint16_t address)
{
	address = address & 0xffff;

	uint32_t temp_address = address;
	terminal->printf("\r.:%04x ", temp_address);
	for (int i=0; i<8; i++) {
		terminal->printf("%02x ", system->core->read8(temp_address));
		temp_address++;
		temp_address &= 0xffff;
	}
	temp_address = address;

	terminal->bg_color = bg_acc;
	terminal->fg_color = fg_acc;

	for (int i=0; i<8; i++) {
		uint8_t temp_byte = system->core->read8(temp_address);
		terminal->putsymbol(temp_byte);
		temp_address++;
		temp_address &= 0xffff;
	}

	terminal->bg_color = bg;
	terminal->fg_color = fg;

	address += 8;
	address &= 0xffff;

	for (int i=0; i<32; i++) {
		terminal->cursor_left();
	}
}

void debugger_t::enter_memory_line(char *buffer)
{
	have_prompt = true;

	uint32_t address;
	uint32_t arg0, arg1, arg2, arg3;
	uint32_t arg4, arg5, arg6, arg7;

	buffer[5]  = '\0';
	buffer[8]  = '\0';
	buffer[11] = '\0';
	buffer[14] = '\0';
	buffer[17] = '\0';
	buffer[20] = '\0';
	buffer[23] = '\0';
	buffer[26] = '\0';
	buffer[29] = '\0';

	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("????");
	} else if (!hex_string_to_int(&buffer[6], &arg0)) {
		terminal->putchar('\r');
		for (int i=0; i<7; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[9], &arg1)) {
		terminal->putchar('\r');
		for (int i=0; i<10; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[12], &arg2)) {
		terminal->putchar('\r');
		for (int i=0; i<13; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[15], &arg3)) {
		terminal->putchar('\r');
		for (int i=0; i<16; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[18], &arg4)) {
		terminal->putchar('\r');
		for (int i=0; i<19; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[21], &arg5)) {
		terminal->putchar('\r');
		for (int i=0; i<22; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[24], &arg6)) {
		terminal->putchar('\r');
		for (int i=0; i<25; i++) terminal->cursor_right();
		terminal->puts("??");
	} else if (!hex_string_to_int(&buffer[27], &arg7)) {
		terminal->putchar('\r');
		for (int i=0; i<28; i++) terminal->cursor_right();
		terminal->puts("??");
	} else {
		uint16_t original_address = address;

		arg0 &= 0xff;
		arg1 &= 0xff;
		arg2 &= 0xff;
		arg3 &= 0xff;
		arg4 &= 0xff;
		arg5 &= 0xff;
		arg6 &= 0xff;
		arg7 &= 0xff;

		system->core->write8(address, (uint8_t)arg0); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg1); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg2); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg3); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg4); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg5); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg6); address +=1; address &= 0xffff;
		system->core->write8(address, (uint8_t)arg7); address +=1; address &= 0xffff;

		terminal->putchar('\r');

		memory_dump(original_address);

		original_address += 8;
		original_address &= 0xffff;
		terminal->printf("\n.:%04x ", original_address);
		have_prompt = false;
	}
}

void debugger_t::vram_dump(uint32_t address, uint32_t width)
{
	address &= VRAM_SIZE_MASK;

	uint32_t temp_address = address;

	terminal->printf(".;%06x.%02x ", temp_address, width);

	for (int i=0; i<width; i++) {
		terminal->printf("%02x ", system->core->blitter->vram[temp_address & VRAM_SIZE_MASK]);
		temp_address++;
		temp_address &= VRAM_SIZE_MASK;
	}

	temp_address = address;

	terminal->bg_color = bg_acc;

	for (int i=0; i<width; i++) {
		terminal->bg_color = system->core->blitter->vram[temp_address];
		terminal->putsymbol(ASCII_SPACE);
		temp_address++;
		temp_address &= VRAM_SIZE_MASK;
	}

	terminal->bg_color = bg;

	for (int i=0; i<4*width; i++) {
		terminal->cursor_left();
	}
}

void debugger_t::vram_binary_dump(uint32_t address, uint32_t width)
{
	address &= VRAM_SIZE_MASK;

	uint32_t temp_address = address;

	terminal->printf(".'%06x.%1x ", temp_address, width);

	for (int i=0; i<width; i++) {
		terminal->printf("%c%c%c%c%c%c%c%c",
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x80 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x40 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x20 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x10 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x08 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x04 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x02 ? '1' : '.',
			system->core->blitter->vram[temp_address & VRAM_SIZE_MASK] & 0x01 ? '1' : '.'
		);
		temp_address++;
		temp_address &= VRAM_SIZE_MASK;
	}

	for (int i=0; i<8*width; i++) {
		terminal->cursor_left();
	}
}

uint32_t debugger_t::disassemble_instruction(uint16_t address)
{
	uint32_t cycles;
	if (system->core->cpu->breakpoint_array[address]) {
		terminal->fg_color = fg_acc;
		terminal->bg_color = bg_acc;
	}
	cycles = system->core->cpu->disassemble_instruction(text_buffer, T_BUFFER_SIZE, address) & 0xffff;
	terminal->printf("%s", text_buffer);
	terminal->fg_color = fg;
	terminal->bg_color = bg;

	terminal->putchar('\r');
	for (int i=0; i<7; i++) terminal->cursor_right();
	return cycles;
}

void debugger_t::enter_assembly_line(char *buffer)
{
	uint32_t word;
	uint32_t address;

	uint32_t arguments[5];

	buffer[5] = '\0';

	if (!hex_string_to_int(&buffer[1], &word)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("????");
	} else {
		address = word;

		uint8_t count{0};
		char old_char;

		for (int i=0; i<5; i++) {
			old_char = buffer[8 + (2 * i)];
			buffer[8 + (2 * i)] = '\0';
			if (hex_string_to_int(&buffer[6 + (2 * i)], &word)) {
				arguments[i] = word;
				count++;
				buffer[8 + (2 * i)] = old_char;
			} else {
				terminal->putchar('\r');
				for (int j=0; j<(7 + (2*i)); j++) terminal->cursor_right();
				terminal->printf("??");
				break;
			}
		}

		for (int i=0; i<count; i++) {
			system->core->write8((address + i) & 0xffff, arguments[i]);
		}
		if (count) {
			terminal->printf("\r.");
			uint8_t no = disassemble_instruction(address);
			terminal->printf("\n.,%04x ", address + no);
		} else {
			if (!count) terminal->printf("\n.");
		}
	}
}

void debugger_t::enter_vram_line(char *buffer)
{
	uint32_t address;
	uint32_t columns;
	uint32_t values[0x10];

	buffer[7] = 0;
	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("??????");
		prompt();
	} else {
		buffer[10] = 0;
		if (!hex_string_to_int(&buffer[8], &columns)) {
			terminal->putchar('\r');
			for (int i=0; i<9; i++) terminal->cursor_right();
			terminal->puts("??");
			prompt();
		} else {
			bool correct = true;
			for (int i=0; i<columns; i++) {
				buffer[13 + (3 * i)] = 0;
				if (!hex_string_to_int(&buffer[11 + (3 * i)], &values[i])) {
					terminal->putchar('\r');
					for (int j=0; j<(12 + (3*i)); j++) terminal->cursor_right();
					terminal->puts("??");
					correct = false;
				}
			}
			if (correct) {
				for (int i=0; i<columns; i++) {
					system->core->blitter->vram[address + i] = values[i];
				}
				terminal->printf("\r");
				vram_dump(address, columns);
				terminal->printf("\n.;%06x.%02x ", (address + columns) & VRAM_SIZE_MASK, columns);
			} else {
				prompt();
			}
		}
	}
}

void debugger_t::enter_vram_binary_line(char *buffer)
{
	uint32_t address;
	uint32_t columns;
//	bool values[0x40];	// max 8 bytes
	uint64_t result{0};	// holds max 8 bytes

	bool correct = true;

	buffer[7] = 0;
	if (!hex_string_to_int(&buffer[1], &address)) {
		terminal->putchar('\r');
		terminal->cursor_right();
		terminal->cursor_right();
		terminal->puts("??????");
		prompt();
	} else {
		buffer[9] = 0;
		if (!hex_string_to_int(&buffer[8], &columns)) {
			terminal->putchar('\r');
			for (int i=0; i<9; i++) terminal->cursor_right();
			terminal->puts("?");
			prompt();
		} else {
			if (columns > 8) columns = 8;
			for (int i=0; i<8*columns; i++) {
				result <<= 1;
				if (buffer[10 + i] == '1') {
					result |= 0b1;
				} else if (buffer[10 + i] != '.') {
					correct = false;
					terminal->putchar('\r');
					for (int j=0; j<(11 + i); j++) terminal->cursor_right();
					terminal->printf("?");
					prompt();
					break;
				}
			}
		}
		if (correct) {
			for (int i=0; i<columns; i++) {
				system->core->blitter->vram[(address + i) & VRAM_SIZE_MASK] = (result >> ((columns - i - 1) * 8)) & 0xff;
			}
			terminal->putchar('\r');
			vram_binary_dump(address, columns);
			terminal->printf("\n.\'%06x.%01x ", (address + columns) & VRAM_SIZE_MASK, columns);
		}
	}
}
