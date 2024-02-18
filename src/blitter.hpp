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
#define	FLAGS1_X_Y_FLIP		0b01000000

/*
 * for both pixels and tiles!!!
 * need to write documentation
 */
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
	 * | | | |     | |
	 * | | | |     | |
	 * | | | |     | +-- Use background color (0 = off, 1 = on)
	 * | | | |     +---- Use foreground color (0 = off, 1 = on)
	 * | | +-+---------- Bits per pixel (0b00 = 1, 0b01 = 2, 0b10 = 4, 0b11 = 8)
	 * +-+-------------- Rom font selection (00 = off, 01 = tiny_font, 10... 11...)
	 *
	 * bits 2 to 5: Reserved
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
	
	uint8_t index{0};
	
	uint8_t color_indices[16] = {
		0x00, 0xff, 0xc6, 0x7e,
		0xde, 0x72, 0x5d, 0xf7,
		0xe2, 0x95, 0xea, 0xfc,
		0xaa, 0xba, 0xaf, 0xfe
	};
};

class blitter_ic {
private:
	// blitter registers
	uint8_t src_surface{0};
	uint8_t dst_surface{0};
	uint8_t tile_surface{0};
	uint8_t draw_color{0};
	int16_t x0{0};
	int16_t y0{0};
	int16_t x1{0};
	int16_t y1{0};
	
	/*
	 * To restrain max no of pixels per frame
	 * At start of frame, set to specific level
	 * e.g. max. 8 times total pixels in display.
	 */
	uint32_t pixel_saldo{0};
	
	/*
	 * Returns number of pixels written
	 */
	uint32_t blit(const surface_t *src, surface_t *dst);
	
	uint8_t *font_4x6;
	void init_font_4x6();
	
	struct color_mode_t {
		uint8_t bits_per_pixel;
		uint8_t pixels_per_byte;
		uint8_t mask;
		bool color_lookup;
	};
	
	const struct color_mode_t color_modes[4] = {
		{ 1, 8, 0b00000001, true  },
		{ 2, 4, 0b00000011, true  },
		{ 4, 2, 0b00001111, true  },
		{ 8, 1, 0b11111111, false }
	};

public:
	blitter_ic();
	~blitter_ic();
	
	surface_t surface[16];
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
	
	uint8_t io_surfaces_read8(uint16_t address);
	void io_surfaces_write8(uint16_t address, uint8_t value);
	
	uint8_t io_color_indices_read8(uint16_t address);
	void io_color_indices_write8(uint16_t address, uint8_t value);
	
//	uint8_t io_palette_read8(uint16_t address) {
//		if (address & 0b1) {
//			return palette[(address & 0x1ff) >> 1] & 0xff;
//		} else {
//			return (palette[(address & 0x1ff) >> 1] & 0xff00) >> 8;
//		}
//	}
	
//	void io_palette_write8(uint16_t address, uint8_t value) {
//		if (address & 0b1) {
//			palette[(address & 0x1ff) >> 1] = (palette[(address & 0x1ff) >> 1] & 0xff00) | value;
//		} else {
//			palette[(address & 0x1ff) >> 1] = (palette[(address & 0x1ff) >> 1] & 0x00ff) | (value << 8);
//		}
//	}
	
	/*
	 * All return number of pixels changed
	 */
	uint32_t clear_surface(const uint8_t surf_no);
	uint32_t blit(const uint8_t s, const uint8_t d);
	uint32_t tile_blit(const uint8_t _s, const uint8_t _d, const uint8_t _ts);
	uint32_t line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);
	uint32_t rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);
	uint32_t fill_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);
//	uint32_t flood_fill(int16_t x0, int16_t y0, uint8_t c, uint8_t d);
	
	void set_pixel_saldo(uint32_t s) { pixel_saldo = s; }
	uint32_t get_pixel_saldo() { return pixel_saldo; }
	
	void update_framebuffer();
	
	uint8_t *vram;
	uint16_t *framebuffer;
	uint16_t *palette;
};

#endif
