#include "common.hpp"
#include "terminal.hpp"
#include <cstdio>
#include <cstdarg>

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
	cursor_pos = 0;
}

void terminal_t::putsymbol_at_cursor(char symbol)
{
	blitter->vram[(ts->base + (0 * characters) + cursor_pos) & VRAM_SIZE_MASK] = symbol;
	blitter->vram[(ts->base + (1 * characters) + cursor_pos) & VRAM_SIZE_MASK] = fg_color;
	blitter->vram[(ts->base + (2 * characters) + cursor_pos) & VRAM_SIZE_MASK] = bg_color;
}

void terminal_t::putsymbol(char symbol)
{
	putsymbol_at_cursor(symbol);
	cursor_pos++;
	if (cursor_pos >= characters) {
		add_bottom_row();
		cursor_pos -= ts->columns;
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
			cursor_pos -= cursor_pos % ts->columns;
			break;
		case '\n':
			cursor_pos -= cursor_pos % ts->columns;
			if ((cursor_pos / ts->columns) == (ts->rows - 1)) {
				add_bottom_row();
			} else {
				cursor_pos += ts->columns;
			}
			break;
		case '\t':
			while ((cursor_pos % ts->columns) & 0b11) {
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
