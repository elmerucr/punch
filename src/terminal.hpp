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
	
	void cursor_left();
	void cursor_right();
	void cursor_up();
	void cursor_down();
	void backspace();
	
	void activate_cursor();
	void deactivate_cursor();
	void process_cursor_state();
	
	void add_bottom_row();
	
	void get_command(char *c, int length);
	
	inline int lines_remaining() {
		return ts->rows - (cursor_position / ts->columns) - 1;
	}
	
	uint8_t fg_color{0b00110100};
	uint8_t bg_color{0b00000000};
private:
	tile_surface *ts;
	blitter_ic *blitter;
	uint16_t characters;
	uint16_t cursor_position{0};
	uint8_t  cursor_interval{20};
	uint8_t  cursor_countdown{0};
	char     cursor_original_char;
	uint16_t cursor_original_color;
	uint16_t cursor_original_background_color;
	bool     cursor_blinking{false};
	
	//char	command_buffer[256];
	//char	*command;
};

#endif