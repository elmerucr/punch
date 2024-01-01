/*
 * blitter.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

#define FLAGS0_ROMFONT		0b01000000

#define	FLAGS1_DBLWIDTH		0b00000001
#define FLAGS1_DBLHEIGHT	0b00000010
#define FLAGS1_HOR_FLIP		0b00010000
#define FLAGS1_VER_FLIP		0b00100000
#define	FLAGS1_XY_FLIP		0b01000000

struct surface_t {
	/*
	 * Properties related to flags_0 (as encoded inside machine)
	 * Color related
	 *
	 * 7 6 5 4 3 2 1 0
	 * | |     | |   |
	 * | |     | |   +-- Keycolor (0 = off, 1 = on)
	 * | |     | +------ Use foreground color (0 = off, 1 = on)
	 * | |     +-------- Use background color (0 = off, 1 = on)
	 * | |
	 * +-+-------------- Rom font selection (00 = off, 01 = tiny_font, 10... 11...)
	 *
	 * bits 3 to 5: Reserved
	 */
	uint8_t flags_0;
	
	/*
	 * Properties related to flags_1 (as encoded inside machine)
	 * Size, flips and xy flip
	 *
	 * 7 6 5 4 3 2 1 0
	 *   | | |     | |
	 *   | | |     | +-- Double width (0 = off, 1 = on)
	 *   | | |     +---- Double height (0 = off, 1 = on)
	 *   | | +---------- Horizontal flip (0 = off, 1 = on)
	 *   | +------------ Vertical flip (0 = off, 1 = on)
	 *   +-------------- XY flip (0 = off, 1 = on)
	 *
	 * bits 2,3 and 7: Reserved
	 */
	uint8_t flags_1;
	
	uint8_t keycolor;
	uint8_t fg_col;
	uint8_t bg_col;
	
	uint8_t index;
	uint32_t base;
	int16_t x;
	int16_t y;
	uint16_t w;
	uint16_t h;
};

struct tile_surface_t {
	uint8_t columns;
	uint8_t rows;
	uint32_t base;
	int16_t x;
	int16_t y;
};

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	tile_surface_t tile_surface[8];
	surface_t surface[8];
	
	surface_t font;
	surface_t turn_text;
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
	
	/*
	 * all return number of pixels changed
	 */
	uint32_t blit(const surface_t *src, surface_t *dst);
	uint32_t blit(const uint8_t s, const uint8_t d);
	uint32_t tile_blit(const tile_surface_t *ts, const surface_t *src, surface_t *dst);
	uint32_t clear_surface(const surface_t *s);
	uint32_t clear_surface(const uint8_t s);
	
//	uint32_t rectangle();
//	uint32_t line();
//	uint32_t circle();
	
	uint8_t *vram;
private:
	uint8_t *font_4x6;
	void init_font_4x6();
};

#endif
