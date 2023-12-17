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

debugger_t::debugger_t(app_t *a)
{
	app = a;
	
	irq_no = app->core->exceptions->connect_device("debugger");
	
	//core = c;
	//keyboard = k;
	
	blitter = new blitter_ic();
	blitter->framebuffer_bank = 0x0e;
	blitter->framebuffer.bg_col = 0b00000000;
	
	blitter->font.flags_0 = 0b01000111;
	blitter->font.flags_1 = 0b00000000;
	blitter->font.keycolor = C64_BLUE;

	character_screen.columns = MAX_PIXELS_PER_SCANLINE / 4;
	character_screen.rows = MAX_SCANLINES / 6;
	character_screen.base = 0x10000;
	character_screen.x = 0;
	character_screen.y = 0;
	
	terminal = new terminal_t(&character_screen, blitter);
	terminal->bg_color = 0b00000000;
	terminal->clear();
	print_version();
	
//	core->cpu->status(text_buffer, 2048);
//	terminal->printf("\n\n%s", text_buffer);
//	core->cpu->disassemble_instruction(text_buffer, core->cpu->get_pc());
//	terminal->printf("\n%s", text_buffer);
	//prompt();
	//terminal->activate_cursor();
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
	terminal->printf("\npunch v%i.%i.%i (C)%i elmerucr",
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
	blitter->clear_surface(&blitter->framebuffer);
	blitter->tile_blit(&character_screen, &blitter->font, &blitter->framebuffer);
}

void debugger_t::run()
{
	uint8_t symbol = 0;
	
	terminal->process_cursor_state();
	
	while (app->keyboard->events_waiting()) {
		terminal->deactivate_cursor();
		symbol = app->keyboard->pop_event();
		switch (symbol) {
			case ASCII_F1:
				app->core->run(0);
				status();
				prompt();
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
				
//				if (*command_buffer == 'r') {
//					terminal->printf("\nrrrrr");
//				}
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
	
	bool have_prompt = true;
	
	char *token0, *token1;
	token0 = strtok(c, " ");
	
	if (token0 == NULL) {
		//have_prompt = false;
	} else if (strcmp(token0, "clear") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "exit") == 0) {
		terminal->printf("\nexit punch (y/n)");
		redraw();
		app->host->update_debugger_texture(&blitter->vram[(blitter->framebuffer_bank & 0x0f) << 16]);
		app->host->update_screen();
		if (app->host->events_yes_no()) {
			app->running = false;
			app->host->events_wait_until_key_released(SDLK_y);
			have_prompt = false;
		} else {
			app->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "irq") == 0) {
		app->core->exceptions->toggle(irq_no);
		status();
	} else if (strcmp(token0, "m") == 0) {
		have_prompt = false;
		token1 = strtok(NULL, " ");
		
		uint8_t lines_remaining = terminal->lines_remaining();
		if (lines_remaining == 0) lines_remaining = 1;
		
		uint32_t temp_pc = app->core->cpu->get_pc();
		
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
		app->core->run(0);
		status();
	} else if (strcmp(token0, "reset") == 0) {
		terminal->printf("\nreset punch (y/n)");
		redraw();
		app->host->update_debugger_texture(&blitter->vram[(blitter->framebuffer_bank & 0x0f) << 16]);
		app->host->update_screen();
		if (app->host->events_yes_no()) {
			app->core->reset();
			app->host->events_wait_until_key_released(SDLK_y);
		} else {
			app->host->events_wait_until_key_released(SDLK_n);
		}
	} else if (strcmp(token0, "run") == 0) {
		have_prompt = false;
		app->switch_to_run_mode();
		app->host->events_wait_until_key_released(SDLK_RETURN);
	} else if (strcmp(token0, "s") == 0) {
		status();
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
	app->core->cpu->status(text_buffer, 2048);
	terminal->printf("%s\n", text_buffer);
	uint16_t pc = app->core->cpu->get_pc();
	for (int i=0; i<8; i++) {
		pc += app->core->cpu->disassemble_instruction(text_buffer, pc);
		terminal->printf("\n%s", text_buffer);
	}
	app->core->cpu->stacks(text_buffer, 2048, 8);
	terminal->printf("\n\n%s", text_buffer);
	app->core->exceptions->status(text_buffer, 2048);
	terminal->printf("\n\n%s", text_buffer);
}

void debugger_t::memory_dump(uint16_t address)
{
	address = address & 0xffff;

	uint32_t temp_address = address;
	terminal->printf("\r.:%04x ", temp_address);
	for (int i=0; i<8; i++) {
		terminal->printf("%02x ", app->core->read8(temp_address));
		temp_address++;
		temp_address &= 0xffff;
	}
	temp_address = address;

	terminal->bg_color = 0b00000100;

	for (int i=0; i<8; i++) {
		uint8_t temp_byte = app->core->read8(temp_address);
		terminal->putsymbol(temp_byte);
		temp_address++;
		temp_address &= 0xffff;
	}

	terminal->bg_color = 0b00000000;

	address += 8;
	address &= 0xffff;
       
	for (int i=0; i<32; i++) {
		terminal->cursor_left();
	}
}