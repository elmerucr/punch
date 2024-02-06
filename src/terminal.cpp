#include "common.hpp"
#include "terminal.hpp"
#include "debugger.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>

terminal_t::terminal_t(system_t *s, tile_surface_t *t, blitter_ic *b)
{
	system = s;
	ts = t;
	blitter = b;
	characters = ts->w * ts->h;
}

void terminal_t::clear()
{
	for (int i=0; i < characters; i++) {
		blitter->vram[((ts->base_page << 8) + (0 * characters) + i) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[((ts->base_page << 8) + (1 * characters) + i) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[((ts->base_page << 8) + (2 * characters) + i) & VRAM_SIZE_MASK] = bg_color;
	}
	cursor_position = 0;
}

void terminal_t::putsymbol_at_cursor(char symbol)
{
	blitter->vram[((ts->base_page << 8) + (0 * characters) + cursor_position) & VRAM_SIZE_MASK] = symbol;
	blitter->vram[((ts->base_page << 8) + (1 * characters) + cursor_position) & VRAM_SIZE_MASK] = fg_color;
	blitter->vram[((ts->base_page << 8) + (2 * characters) + cursor_position) & VRAM_SIZE_MASK] = bg_color;
}

void terminal_t::putsymbol(char symbol)
{
	putsymbol_at_cursor(symbol);
	cursor_position++;
	if (cursor_position >= characters) {
		add_bottom_row();
		cursor_position -= ts->w;
	}
}

void terminal_t::add_bottom_row()
{
	uint16_t no_of_tiles_to_move = characters - ts->w;

	for (int i=0; i < no_of_tiles_to_move; i++) {
		blitter->vram[((ts->base_page << 8) + (0 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + (0 * characters) + ts->w + i) & VRAM_SIZE_MASK];
		blitter->vram[((ts->base_page << 8) + (1 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + (1 * characters) + ts->w + i) & VRAM_SIZE_MASK];
		blitter->vram[((ts->base_page << 8) + (2 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + (2 * characters) + ts->w + i) & VRAM_SIZE_MASK];
	}
	for (int i=no_of_tiles_to_move; i < characters; i++) {
		blitter->vram[((ts->base_page << 8) + (0 * characters) + i) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[((ts->base_page << 8) + (1 * characters) + i) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[((ts->base_page << 8) + (2 * characters) + i) & VRAM_SIZE_MASK] = bg_color;
	}
}

int terminal_t::putchar(int character)
{
	uint8_t result = (uint8_t)character;
	switch (result) {
		case '\r':
			cursor_position -= cursor_position % ts->w;
			break;
		case '\n':
			cursor_position -= cursor_position % ts->w;
			if ((cursor_position / ts->w) == (ts->h - 1)) {
				add_bottom_row();
			} else {
				cursor_position += ts->w;
			}
			break;
		case '\t':
			while ((cursor_position % ts->w) & 0b11) {
				putsymbol(' ');
			}
			break;
		default:
			putsymbol(result);
			break;
	}
	return result;
}

int terminal_t::puts(const char *text)
{
	int char_count = 0;
	if (text) {
		while (*text) {
			putchar(*text);
			char_count++;
			text++;
		}
	}
	return char_count;
}

int terminal_t::printf(const char *format, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, format);
	int number = vsnprintf(buffer, 1024, format, args);
	va_end(args);
	puts(buffer);
	return number;
}

void terminal_t::activate_cursor()
{
	cursor_original_char = blitter->vram[((ts->base_page << 8) + cursor_position) & VRAM_SIZE_MASK];
	cursor_original_color = blitter->vram[((ts->base_page << 8) + characters + cursor_position) & VRAM_SIZE_MASK];
	cursor_original_background_color = blitter->vram[((ts->base_page << 8) + (2 * characters) + cursor_position) & VRAM_SIZE_MASK];
	cursor_blinking = true;
	cursor_countdown = 0;
}

void terminal_t::deactivate_cursor()
{
	cursor_blinking = false;
	blitter->vram[((ts->base_page << 8) + cursor_position) & VRAM_SIZE_MASK] = cursor_original_char;
	blitter->vram[((ts->base_page << 8) + characters + cursor_position) & VRAM_SIZE_MASK] = cursor_original_color;
	blitter->vram[((ts->base_page << 8) + (2 * characters) + cursor_position) & VRAM_SIZE_MASK] = cursor_original_background_color;
}

void terminal_t::process_cursor_state()
{
	if (cursor_blinking == true) {
		if (cursor_countdown == 0) {
			blitter->vram[((ts->base_page << 8) + cursor_position) & VRAM_SIZE_MASK] ^= 0x80;
			if (((blitter->vram[((ts->base_page << 8) + cursor_position) & VRAM_SIZE_MASK]) & 0x80) != (cursor_original_char & 0x80)) {
				blitter->vram[((ts->base_page << 8) + characters + cursor_position) & VRAM_SIZE_MASK] = fg_color;
			} else {
				blitter->vram[((ts->base_page << 8) + characters + cursor_position) & VRAM_SIZE_MASK] = cursor_original_color;
			}
			cursor_countdown += cursor_interval;
		}
		cursor_countdown--;
	}
}

void terminal_t::cursor_left()
{
	//uint16_t min_pos = 0;
	if (cursor_position > 0) cursor_position--;
}

void terminal_t::cursor_right()
{
	cursor_position++;
	if (cursor_position > characters - 1) {
		add_bottom_row();
		cursor_position -= ts->w;
	}
}

void terminal_t::cursor_up()
{
	cursor_position -= ts->w;

	if (cursor_position >= characters) {
		cursor_position += ts->w;
		uint32_t address;

		switch (check_output(true, &address)) {
			case NOTHING:
				break;
			case MEMORY:
				add_top_row();
				system->debugger->memory_dump((address - 8) & 0xffff);
				break;
		}
	}
}

void terminal_t::cursor_down()
{
	cursor_position += ts->w;

	// cursor out of current screen?
	if (cursor_position >= characters) {
		//add_bottom_row();
		cursor_position -= ts->w;
		
		uint32_t address = 0;

		switch (check_output(false, &address)) {
			case NOTHING:
				add_bottom_row();
				break;
			case MEMORY:
				add_bottom_row();
				system->debugger->memory_dump((address + 8) & 0xffff);
				break;
		}
	}
}

void terminal_t::backspace()
{
	uint16_t pos = cursor_position;
	uint16_t min_pos = 0;

	if (pos > min_pos) {
		cursor_position--;
		while (pos % ts->w) {
			blitter->vram[((ts->base_page << 8) + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[((ts->base_page << 8) + pos) & VRAM_SIZE_MASK];
			blitter->vram[((ts->base_page << 8) + characters + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[((ts->base_page << 8) + characters + pos) & VRAM_SIZE_MASK];
			blitter->vram[((ts->base_page << 8) + (2 * characters) + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[((ts->base_page << 8) + (2 * characters) + pos) & VRAM_SIZE_MASK];
			pos++;
		}
		blitter->vram[((ts->base_page << 8) + pos - 1) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[((ts->base_page << 8) + characters + pos - 1) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[((ts->base_page << 8) + (2 * characters) + pos - 1) & VRAM_SIZE_MASK] = bg_color;
	}
}

///*
// * https://gist.github.com/kenkam/790090
// */
//inline char *trim_space(char *str)
//{
//	char *end;
//	/* skip leading whitespace */
//	while (*str == ASCII_SPACE) {
//		str = str + 1;
//	}
//	/* remove trailing whitespace */
//	end = str + strlen(str) - 1;
//	while (end > str && (*end == ASCII_SPACE)) {
//		end = end - 1;
//	}
//	/* write null character */
//	*(end+1) = '\0';
//	return str;
//}

void terminal_t::get_command(char *c, int l)
{
	int length = ts->w < (l-1) ? ts->w : (l-1);
	
	uint16_t pos = cursor_position - (cursor_position % ts->w);
	
	for (int i=0; i<length; i++) {
		c[i] = blitter->vram[(ts->base_page << 8) + pos + i];
	}
	c[length] = 0;
	
	//command = command_buffer;
	
	//command = trim_space(command);
	
//	while (*command == '.') command++;
}

enum output_type terminal_t::check_output(bool top_down, uint32_t *address)
{
	enum output_type output = NOTHING;

	for (int i = 0; i < characters; i += ts->w) {
		if (blitter->vram[((ts->base_page << 8) + i + 1)] == ':') {
			output = MEMORY;
			char potential_address[5];
			for (int j=0; j<4; j++) {
				potential_address[j] = blitter->vram[((ts->base_page << 8) + i + 2 + j)];
			}
			potential_address[4] = 0;
			system->debugger->hex_string_to_int(potential_address, address);
			if (top_down) break;
		}
	}
	return output;
}

void terminal_t::add_top_row()
{
	uint16_t no_of_tiles_to_move = characters - ts->w;
	
	uint16_t last_tile = characters - 1;

	for (int i=0; i < no_of_tiles_to_move; i++) {
		blitter->vram[((ts->base_page << 8) + last_tile + (0 * characters) - i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + last_tile + (0 * characters) - ts->w - i) & VRAM_SIZE_MASK];
		blitter->vram[((ts->base_page << 8) + last_tile + (1 * characters) - i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + last_tile + (1 * characters) - ts->w - i) & VRAM_SIZE_MASK];
		blitter->vram[((ts->base_page << 8) + last_tile + (2 * characters) - i) & VRAM_SIZE_MASK] =
			blitter->vram[((ts->base_page << 8) + last_tile + (2 * characters) - ts->w - i) & VRAM_SIZE_MASK];
	}
	for (int i=0; i < ts->w; i++) {
		blitter->vram[((ts->base_page << 8) + (0 * characters) + i) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[((ts->base_page << 8) + (1 * characters) + i) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[((ts->base_page << 8) + (2 * characters) + i) & VRAM_SIZE_MASK] = bg_color;
	}
}
