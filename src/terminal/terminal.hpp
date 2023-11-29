#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include "blitter.hpp"

class terminal_t {
public:
	terminal_t(tile_surface *t, blitter_ic *b);
	
	void clear();
	void putsymbol_at_cursor(char symbol);
	void putsymbol(char symbol);
	int putchar(int character);
	int puts(const char *text);
	int printf(const char *format, ...);
	
	void add_bottom_row();
	
private:
	tile_surface *ts;
	blitter_ic *blitter;
	uint16_t characters;
	uint16_t cursor_pos{0};
	
	uint8_t fg_color{0b00111000};
	uint8_t bg_color{0b00000000};
};

#endif
