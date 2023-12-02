#include "common.hpp"
#include "terminal.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>

terminal_t::terminal_t(tile_surface *t, blitter_ic *b)
{
	ts = t;
	blitter = b;
	characters = ts->columns * ts->rows;
}

void terminal_t::clear()
{
	for (int i=0; i < characters; i++) {
		blitter->vram[(ts->base + (0 * characters) + i) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[(ts->base + (1 * characters) + i) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[(ts->base + (2 * characters) + i) & VRAM_SIZE_MASK] = bg_color;
	}
	cursor_position = 0;
}

void terminal_t::putsymbol_at_cursor(char symbol)
{
	blitter->vram[(ts->base + (0 * characters) + cursor_position) & VRAM_SIZE_MASK] = symbol;
	blitter->vram[(ts->base + (1 * characters) + cursor_position) & VRAM_SIZE_MASK] = fg_color;
	blitter->vram[(ts->base + (2 * characters) + cursor_position) & VRAM_SIZE_MASK] = bg_color;
}

void terminal_t::putsymbol(char symbol)
{
	putsymbol_at_cursor(symbol);
	cursor_position++;
	if (cursor_position >= characters) {
		add_bottom_row();
		cursor_position -= ts->columns;
	}
}

void terminal_t::add_bottom_row()
{
	uint16_t no_of_tiles_to_move = characters - ts->columns;

	for (int i=0; i < no_of_tiles_to_move; i++) {
		blitter->vram[(ts->base + (0 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[(ts->base + (0 * characters) + ts->columns + i) & VRAM_SIZE_MASK];
		blitter->vram[(ts->base + (1 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[(ts->base + (1 * characters) + ts->columns + i) & VRAM_SIZE_MASK];
		blitter->vram[(ts->base + (2 * characters) + i) & VRAM_SIZE_MASK] =
			blitter->vram[(ts->base + (2 * characters) + ts->columns + i) & VRAM_SIZE_MASK];
	}
	for (int i=no_of_tiles_to_move; i < characters; i++) {
		blitter->vram[(ts->base + (0 * characters) + i) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[(ts->base + (1 * characters) + i) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[(ts->base + (2 * characters) + i) & VRAM_SIZE_MASK] = bg_color;
	}
}

int terminal_t::putchar(int character)
{
	uint8_t result = (uint8_t)character;
	switch (result) {
		case '\r':
			cursor_position -= cursor_position % ts->columns;
			break;
		case '\n':
			cursor_position -= cursor_position % ts->columns;
			if ((cursor_position / ts->columns) == (ts->rows - 1)) {
				add_bottom_row();
			} else {
				cursor_position += ts->columns;
			}
			break;
		case '\t':
			while ((cursor_position % ts->columns) & 0b11) {
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
	//blitter->vram[ts->base + cursor_position] ^= 0x80;
	cursor_original_char = blitter->vram[(ts->base + cursor_position) & VRAM_SIZE_MASK];
	cursor_original_color = blitter->vram[(ts->base + characters + cursor_position) & VRAM_SIZE_MASK];
	cursor_original_background_color = blitter->vram[(ts->base + (2 * characters) + cursor_position) & VRAM_SIZE_MASK];
//	blit[number].cursor_original_char = terminal_get_tile(number, blit[number].cursor_position);
//	blit[number].cursor_original_color = terminal_get_tile_fg_color(number, blit[number].cursor_position);
//	blit[number].cursor_original_background_color = terminal_get_tile_bg_color(number, blit[number].cursor_position);
	cursor_blinking = true;
	cursor_countdown = 0;
}

void terminal_t::deactivate_cursor()
{
	//blitter->vram[(ts->base + cursor_position) & VRAM_SIZE_MASK] ^= 0x80;
	cursor_blinking = false;
	blitter->vram[(ts->base + cursor_position) & VRAM_SIZE_MASK] = cursor_original_char;
	blitter->vram[(ts->base + characters + cursor_position) & VRAM_SIZE_MASK] = cursor_original_color;
	blitter->vram[(ts->base + (2 * characters) + cursor_position) & VRAM_SIZE_MASK] = cursor_original_background_color;
//	terminal_set_tile(no, blit[no].cursor_position, blit[no].cursor_original_char);
//	terminal_set_tile_fg_color(no, blit[no].cursor_position, blit[no].cursor_original_color);
//	terminal_set_tile_bg_color(no, blit[no].cursor_position, blit[no].cursor_original_background_color);
}

void terminal_t::process_cursor_state()
{
	if (cursor_blinking == true) {
		if (cursor_countdown == 0) {
			blitter->vram[(ts->base + cursor_position) & VRAM_SIZE_MASK] ^= 0x80;
			if (((blitter->vram[(ts->base + cursor_position) & VRAM_SIZE_MASK]) & 0x80) != (cursor_original_char & 0x80)) {
				blitter->vram[(ts->base + characters + cursor_position) & VRAM_SIZE_MASK] = fg_color;
			} else {
				blitter->vram[(ts->base + characters + cursor_position) & VRAM_SIZE_MASK] = cursor_original_color;
			}
//			if ((terminal_get_tile(no, blit[no].cursor_position) & 0x80) != (blit[no].cursor_original_char & 0x80)) {
//				terminal_set_tile_fg_color(no, blit[no].cursor_position, blit[no].foreground_color);
//			} else {
//				terminal_set_tile_fg_color(no, blit[no].cursor_position, blit[no].cursor_original_color);
//			}
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
		cursor_position -= ts->columns;
	}
}

void terminal_t::cursor_up()
{
	cursor_position -= ts->columns;

	if (cursor_position >= characters) {
		cursor_position += ts->columns;
//		uint32_t address;
//
//		switch (terminal_check_output(no, true, &address)) {
//			case E64::NOTHING:
//				terminal_add_top_row(no);
//				break;
//			case E64::ASCII:
//				terminal_add_top_row(no);
//				//hud.memory_dump((address-8) & (RAM_SIZE_CPU_VISIBLE - 1), 1);
//				break;
//			case E64::MONITOR_WORD:
//				terminal_add_top_row(no);
//				//hud.memory_word_dump((address - 16) & 0xfffffe, 1);
//				break;
//		}
	}
}

void terminal_t::cursor_down()
{
	cursor_position += ts->columns;

	// cursor out of current screen?
	if (cursor_position >= characters) {
		add_bottom_row();
		cursor_position -= ts->columns;
//		uint32_t address;
//
//		switch (terminal_check_output(no, false, &address)) {
//			case E64::NOTHING:
//				terminal_add_bottom_row(no);
//				blit[no].cursor_position -= blit[no].get_columns();
//				break;
//			case E64::ASCII:
//				terminal_add_bottom_row(no);
//				blit[no].cursor_position -= blit[no].get_columns();
//				//hud.memory_dump((address+8) & (RAM_SIZE_CPU_VISIBLE - 1), 1);
//				break;
//			case E64::MONITOR_WORD:
//				terminal_add_bottom_row(no);
//				blit[no].cursor_position -= blit[no].get_columns();
//				//hud.memory_word_dump((address + 16) & 0xfffffe, 1);
//				break;
//		}
	}
}

void terminal_t::backspace()
{
	uint16_t pos = cursor_position;
	uint16_t min_pos = 0;

	if (pos > min_pos) {
		cursor_position--;
		while (pos % ts->columns) {
			blitter->vram[(ts->base + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[(ts->base + pos) & VRAM_SIZE_MASK];
			blitter->vram[(ts->base + characters + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[(ts->base + characters + pos) & VRAM_SIZE_MASK];
			blitter->vram[(ts->base + (2 * characters) + pos - 1) & VRAM_SIZE_MASK] =
				blitter->vram[(ts->base + (2 * characters) + pos) & VRAM_SIZE_MASK];
			pos++;
		}
		blitter->vram[(ts->base + pos - 1) & VRAM_SIZE_MASK] = ' ';
		blitter->vram[(ts->base + characters + pos - 1) & VRAM_SIZE_MASK] = fg_color;
		blitter->vram[(ts->base + (2 * characters) + pos - 1) & VRAM_SIZE_MASK] = bg_color;
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
	int length = ts->columns < (l-1) ? ts->columns : (l-1);
	
	uint16_t pos = cursor_position - (cursor_position % ts->columns);
	
	for (int i=0; i<length; i++) {
		c[i] = blitter->vram[ts->base + pos + i];
	}
	c[length] = 0;
	
	//command = command_buffer;
	
	//command = trim_space(command);
	
//	while (*command == '.') command++;
}
