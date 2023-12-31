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
	
	//core = c;
	//keyboard = k;
	
	blitter = new blitter_ic();
	
	/* framebuffer */
	framebuffer.base = FRAMEBUFFER;
	framebuffer.w = MAX_PIXELS_PER_SCANLINE;
	framebuffer.h = MAX_SCANLINES;
	framebuffer.bg_col = 0b00000000;
	
	blitter->font.flags_0 = 0b01001101;
	blitter->font.flags_1 = 0b00000000;
	blitter->font.keycolor = C64_BLUE;

	character_screen.columns = MAX_PIXELS_PER_SCANLINE / 4;
	character_screen.rows = MAX_SCANLINES / 6;
	character_screen.base = 0x10000;
	character_screen.x = 0;
	character_screen.y = 0;
	
	terminal = new terminal_t(system, &character_screen, blitter);
	terminal->fg_color = fg;
	terminal->bg_color = bg;
	terminal->clear();
	print_version();
	
	for (int i=0; i<(3*21*8); i++) {
		blitter->vram[0x300+i] = bruce_data[i];
	}
	bruce.index = 2;
	bruce.base = 0x300;
	bruce.keycolor = 0x01;
	bruce.flags_0 = 0b00000001;
	bruce.flags_1 = 0b00000001;
	bruce.w = 8;
	bruce.h = 21;
	bruce.x = 130;
	bruce.y = 159;
}

void debugger_t::print_version()
{
	const uint8_t symbol[19] = {
		ASCII_LF,
		0x00, 0x08, 0x0c, 0x00, ASCII_LF,
		0x08, 0x01, 0x00, 0x09, ASCII_LF,
		0x02, 0x04, 0x00, 0x06 ,ASCII_LF,
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
	blitter->clear_surface(&framebuffer);
	blitter->tile_blit(&character_screen, &blitter->font, &framebuffer);
	
	static int state = 0;
	static int wait = 100;
	
	//bruce.index = state & 0b100 ? 1 : 2;
	
	if (wait < 200) {
		bruce.index = 0;
		state = 0;
	} else {
		if (state > 4) {
			bruce.index = 1;
			bruce.x += 2; if (bruce.x > 340) bruce.x = -20;
		} else {
			bruce.index = 2;
			bruce.x += 2; if (bruce.x > 340) bruce.x = -20;
		}
		
		state++; if (state == 8) state = 0;
	}
	wait++; if (wait >300) wait = 0;
	
	blitter->blit(&bruce, &framebuffer);
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
			case ASCII_F3:
				terminal->deactivate_cursor();
				terminal->printf("run");
				system->switch_to_run_mode();
				system->host->events_wait_until_key_released(SDLK_F3);
				break;
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
	
	char *token0, *token1;
	token0 = strtok(c, " ");
	
	if (token0 == NULL) {
		//have_prompt = false;
	} else if (token0[0] == ':') {
		have_prompt = false;
		enter_memory_line(c);
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
	} else if (strcmp(token0, "clear") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "exit") == 0) {
		terminal->printf("\nexit punch (y/n)");
		redraw();
		blitter->update_framebuffer();
		system->host->update_debugger_texture(blitter->framebuffer);
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
	} else if (strcmp(token0, "n") == 0) {
		system->core->run(true);
		status();
	} else if (strcmp(token0, "palette") == 0) {
		terminal->printf("\n    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
		for (int i=0; i<256; i++) {
			if ((i % 16) == 0) {
				terminal->fg_color = fg;
				terminal->printf("\n ");
				if (((i & 0xf0) >> 4) < 10) {
					terminal->putsymbol(0x30 + ((i & 0xf0) >> 4));
				} else {
					terminal->putsymbol(0x57 + ((i & 0xf0) >> 4));
				}
				terminal->putchar(' ');
			}
			terminal->fg_color = i;
			terminal->putsymbol(0xa0);
			terminal->putsymbol(0xa0);
			terminal->putsymbol(0xa0);
		}
		terminal->fg_color = fg;
	} else if (strcmp(token0, "reset") == 0) {
		terminal->printf("\nreset punch (y/n)");
		redraw();
		blitter->update_framebuffer();
		system->host->update_debugger_texture(blitter->framebuffer);
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
	terminal->printf("__cpu__________________________________________________________");
	system->core->cpu->status(text_buffer, 2048);
	terminal->printf("\n%s", text_buffer);
	terminal->printf("\n\n__disassembly__________________________________");
	uint16_t pc = system->core->cpu->get_pc();
	for (int i=0; i<8; i++) {
		if (system->core->cpu->breakpoint_array[pc]) {
			terminal->fg_color = fg_acc;
		}
		pc += system->core->cpu->disassemble_instruction(text_buffer, pc);
		terminal->printf("\n%s", text_buffer);
		terminal->fg_color = fg;
	}
	
	terminal->printf("\n\n__usp______ssp_____   _t__cr__sr___bpm______cycles_  _IRQ_State__Name______");
	
	for (int i=0; i<8; i++) {
		uint16_t usp = (system->core->cpu->get_us() + i) & 0xffff;
		uint8_t usp_b = system->core->read8(usp);
		uint16_t ssp = (system->core->cpu->get_sp() + i) & 0xffff;
		uint8_t ssp_b = system->core->read8(ssp);
		
		system->core->exceptions->status(text_buffer, 2048, i);
		
		terminal->printf("\n %04x %02x  %04x %02x      %u %s %s %5u  %10u     %s",
				 usp,
				 usp_b,
				 ssp,
				 ssp_b,
				 i,
				 system->core->timer->io_read_byte(0x1) & (1 << i) ? " on" : "off",
				 system->core->timer->io_read_byte(0x0) & (1 << i) ? "irq" : "   ",
				 system->core->timer->get_timer_bpm(i),
				 system->core->timer->get_timer_clock_interval(i) - system->core->timer->get_timer_counter(i),
				 text_buffer);
	}

	terminal->printf("\n\n%i of %i frame cycles done", system->core->get_cpu_cycle_saldo(), CPU_CYCLES_PER_FRAME);
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

	for (int i=0; i<8; i++) {
		uint8_t temp_byte = system->core->read8(temp_address);
		terminal->putsymbol(temp_byte);
		temp_address++;
		temp_address &= 0xffff;
	}

	terminal->bg_color = bg;

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
