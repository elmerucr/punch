/*
 * blitter.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>
#include "common.hpp"
#include "font_4x6.hpp"
#include "font_cbm_8x8.hpp"

#define FLAGS0_NOFONT		0b00000000
#define FLAGS0_TINYFONT		0b01000000

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

	uint32_t base_address{0};

	/*
	 * Properties related to flags_0 (as encoded inside machine)
	 *
	 * 7 6 5 4 3 2 1 0
	 *   | | |     | |
	 *   | | |     | |
	 *   | | |     | +-- Use background color (0 = off, 1 = on)
	 *   | | |     +---- Use foreground color (0 = off, 1 = on)
	 *   +-+-+---------- Bits per pixel (0b000 = 1, 0b001 = 2, 0b010 = 4, 0b011 = 8, 0b1xx = 16)
	 *
	 *
	 * bits 2, 3, 6 and 7: Reserved
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

	/*
	 * Properties related to flags_2 (as encoded inside machine)
	 *
	 * 7 6 5 4 3 2 1 0
	 *           | | |
	 *           +-+-+-- Rom font selection
	 *                    000 = off
	 *                    001 = tiny font 4x6
	 *                    010 = off (reserved)
	 *                    011 = off (reserved)
	 *                    100 = cbm font 8x8
	 *                    101 = off (reserved)
	 *                    110 = off (reserved)
	 *                    111 = off (reserved)
	 *
	 * bits 3, 4, 5, 6, 7: reserved
	 */
	uint8_t flags_2{0};

	uint8_t index{0};

	/*
	 * Default 16 colors at init, resemble c64
	 */
	uint8_t color_indices[16] = {
		PUNCH_BLACK, PUNCH_WHITE, PUNCH_RED, PUNCH_CYAN,
		PUNCH_PURPLE, PUNCH_GREEN, PUNCH_BLUE, PUNCH_YELLOW,
		PUNCH_ORANGE, PUNCH_BROWN, PUNCH_LIGHTRED, PUNCH_DARKGREY,
		PUNCH_GREY, PUNCH_LIGHTGREEN, PUNCH_LIGHTBLUE, PUNCH_LIGHTGREY
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

	font_4x6_t font_4x6;
	font_cbm_8x8_t font_cbm_8x8;

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

	void reset();

	surface_t surface[16];

	inline uint8_t vram_read8(uint32_t address)
	{
		if (address & 0b1) {
			return _vram[address >> 1] & 0xff;
		} else {
			return _vram[address >> 1] >> 8;
		}
	}

	inline void vram_write8(uint32_t address, uint8_t value)
	{
		if (address & 0b1) {
			_vram[address >> 1] = (_vram[address >> 1] & 0xff00) | value;
		} else {
			_vram[address >> 1] = (_vram[address >> 1] & 0x00ff) | (value << 8);
		}
	}

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	uint8_t io_surfaces_read8(uint16_t address);
	void io_surfaces_write8(uint16_t address, uint8_t value);

	uint8_t io_color_indices_read8(uint16_t address);
	void io_color_indices_write8(uint16_t address, uint8_t value);

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
	uint32_t tile_blit(const uint8_t s, const uint8_t d, const uint8_t _ts);
	uint32_t clear_surface(const uint8_t col, const uint8_t dest);
	uint32_t pset(int16_t x0, int16_t y0, uint8_t c, uint8_t d);
	uint32_t line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);
	uint32_t rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);
	uint32_t solid_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d);

	void set_pixel_saldo(uint32_t s) { pixel_saldo = s; }
	uint32_t get_pixel_saldo() { return pixel_saldo; }

	void update_framebuffer(uint32_t base_address);

	uint8_t *_vram;
	uint16_t *framebuffer;
	uint16_t *palette;
};

#endif
