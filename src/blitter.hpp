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
	int16_t x{0};
	int16_t y{0};

	uint16_t w{0};
	uint16_t h{0};
	
	uint16_t base_page{0};
	
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
	uint8_t flags_0{0};
	
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
	uint8_t flags_1{0};
	
	uint8_t fg_col{0};
	uint8_t bg_col{0};
	
	uint8_t keycolor{0};
	
	uint8_t index{0};
};

struct tile_surface_t {
	int16_t x{0};
	int16_t y{0};

	uint16_t w{0};
	uint16_t h{0};

	uint16_t base_page{0};
};

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	tile_surface_t tile_surface[8];
	surface_t surface[8];
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
	
	uint8_t io_palette_read8(uint16_t address) {
		if (address & 0b1) {
			return palette[(address & 0x1ff) >> 1] & 0xff;
		} else {
			return (palette[(address & 0x1ff) >> 1] & 0xff00) >> 8;
		}
	}
	
	void io_palette_write8(uint16_t address, uint8_t value) {
		if (address & 0b1) {
			palette[(address & 0x1ff) >> 1] = (palette[(address & 0x1ff) >> 1] & 0xff00) | value;
		} else {
			palette[(address & 0x1ff) >> 1] = (palette[(address & 0x1ff) >> 1] & 0x00ff) | (value << 8);
		}
	}
	
	/*
	 * All return number of pixels changed
	 */
	uint32_t blit(const uint8_t s, const uint8_t d);
	uint32_t blit(const surface_t *src, surface_t *dst);
	
	uint32_t tile_blit(const uint8_t s, const uint8_t d, const uint8_t ts);
	uint32_t tile_blit(const surface_t *src, surface_t *dst, const tile_surface_t *ts);
	
	uint32_t clear_surface(const uint8_t s);
	uint32_t clear_surface(const surface_t *s);
	
	void set_pixel_saldo(uint32_t s) { pixel_saldo = s; }
	uint32_t get_pixel_saldo() { return pixel_saldo; }
	
	void update_framebuffer();
	
//	uint32_t rectangle();
//	uint32_t line();
//	uint32_t circle();
	
	uint8_t *vram;
	uint16_t *framebuffer;
	uint16_t *palette;
private:
	uint8_t index0{0};
	uint8_t index1{0};
	uint8_t index2{0};
	uint8_t index3{0};
	
	/*
	 * To restrain max no of pixels per frame
	 * At start of frame, set to specific level
	 * e.g. max. 8 times total pixels in display.
	 */
	uint32_t pixel_saldo{0};
	
	uint8_t *font_4x6;
	void init_font_4x6();
};

#endif
