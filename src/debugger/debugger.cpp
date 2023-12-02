#include "debugger.hpp"
#include "common.hpp"
#include <cstdio>

debugger_t::debugger_t(core_t *c, keyboard_t *k)
{
	core = c;
	keyboard = k;
	
	blitter = new blitter_ic();
	blitter->framebuffer_bank = 0x0e;
	blitter->framebuffer.bg_col = 0b00000000;
	
	blitter->font.flags_0 = 0b01000011;
	blitter->font.flags_1 = 0b00000000;
	blitter->font.keycolor = C64_BLUE;

	character_screen.columns = MAX_PIXELS_PER_SCANLINE / 4;
	character_screen.rows = MAX_SCANLINES / 6;
	character_screen.base = 0x10000;
	character_screen.x = 0;
	character_screen.y = 0;
	
	terminal = new terminal_t(&character_screen, blitter);
	terminal->clear();
	print_version();
	
	core->cpu->status(text_buffer, 2048);
	terminal->printf("\n\n%s", text_buffer);
	prompt();
	terminal->activate_cursor();
}

void debugger_t::print_version()
{
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
	
	while (keyboard->events_waiting()) {
		terminal->deactivate_cursor();
		symbol = keyboard->pop_event();
		switch (symbol) {
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
	while ((*c == ' ') || (*c == '.')) c++;
	
	bool have_prompt = true;
	
	char *token0, *token1;
	token0 = strtok(c, " ");
	
	if (token0 == NULL) {
		//have_prompt = false;
	} else if (strcmp(token0, "clear") == 0) {
		terminal->clear();
	} else if (strcmp(token0, "r") == 0) {
		core->cpu->status(text_buffer, 2048);
		terminal->printf("\n%s", text_buffer);
	} else if (strcmp(token0, "v") == 0) {
		print_version();
	}
	
	if (have_prompt) prompt();
}

void debugger_t::prompt()
{
	terminal->printf("\n.");
}
