// ---------------------------------------------------------------------
// blitter.hpp
// punch
//
// Copyright © 2023-2024 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>
#include <cstddef>
#include "common.hpp"
#include "font_4x6.hpp"
#include "font_cbm_8x8.hpp"

#define FLAGS0_NOFONT		0b00000000
#define FLAGS0_TINYFONT		0b01000000

#define	FLAGS1_DBLWIDTH		0b00000011
#define FLAGS1_DBLHEIGHT	0b00001100
#define FLAGS1_HOR_FLIP		0b00010000
#define FLAGS1_VER_FLIP		0b00100000
#define	FLAGS1_X_Y_FLIP		0b01000000

// for both pixels and tiles!!!
// need to write documentation
struct surface_t {
	int16_t x{0};
	int16_t y{0};

	uint16_t w{0};
	uint16_t h{0};

	uint32_t base_address{0};

	// TODO: fg bg color stuff gone?

	// -----------------------------------------------------------------
	// Properties related to flags_0 (as encoded inside machine)
	//
	// 7 6 5 4 3 2 1 0
	//   | | |     | |
	//   | | |     | |
	//   | | |     | +-- Tile_blit only: Use fixed background color (0 = off, 1 = on)
	//   | | |     +---- Tile_blit only: Use fixed foreground color (0 = off, 1 = on)
	//   +-+-+---------- Bits per pixel (0b000 = 1, 0b001 = 2, 0b010 = 4, 0b011 = 8, 0b100 = 32)
	//
	// bits 2, 3, 6 and 7: Reserved
	// -----------------------------------------------------------------
	uint8_t flags_0{0};

	/*
	 * Properties related to flags_1 (as encoded inside machine)
	 * Size, flips and xy flip
	 *
	 * 7 6 5 4 3 2 1 0
	 *   | | | | | | |
	 *   | | | | | +-+-- Width (00 = 1x, 01 = 2x, 10 = 4x, 11 = 8x)
	 *   | | | +-+------ Height (00 = 1x, 01 = 2x, 10 = 4x, 11 = 8x)
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

	/*
	 * Index is a pointer to a specific tile/sprite/character
	 */
	uint8_t index{0};

	/*
	 * Default color_table for 1, 2, 4 and 8 bit modes at init
	 *
	 * 1 bit uses slots 0 and 1
	 * 2 bit uses slots 0, 1, 2 and 3
	 * 4 bit uses slots 0, ..., 15
	 * 8 bit uses all
	 *
	 */
	uint8_t color_table[256];
};

class blitter_ic {
private:
	/*
	 * Blitter registers
	 */
	uint8_t src_surface{0};
	uint8_t dst_surface{0};
	uint8_t tile_surface{0};
	uint8_t draw_color{0};
	int16_t x0{0};
	int16_t y0{0};
	int16_t x1{0};
	int16_t y1{0};
	uint8_t alpha{255};
	uint8_t gamma_red{255};
	uint8_t gamma_green{255};
	uint8_t gamma_blue{255};
	uint32_t vram_peek{0};	// base address for vram peek page

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

	struct indexed_color_mode_t {
		uint8_t bits_per_pixel;
		uint8_t pixels_per_byte;
		uint8_t mask;
	};

	const struct indexed_color_mode_t indexed_color_modes[4] = {
		{ 1, 8, 0b00000001 },
		{ 2, 4, 0b00000011 },
		{ 4, 2, 0b00001111 },
		{ 8, 1, 0b11111111 }
	};

	/*
	 * The palette is directly stored in main vram
	 */
	const uint32_t palette_addr = 0xc00;

	const uint32_t draw_color_addr = 0xf3e800;

public:
	blitter_ic();
	~blitter_ic();

	void reset();

	surface_t surface[16];

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	uint8_t io_surfaces_read8(uint16_t address);
	void io_surfaces_write8(uint16_t address, uint8_t value);

	uint8_t io_color_table_read8(uint16_t address);
	void io_color_table_write8(uint16_t address, uint8_t value);

	inline void blend(uint32_t s, uint32_t d)
	{
		/*
		 * Force to 32 bit boundaries
		 */
		s &= 0xfffffc;
		d &= 0xfffffc;

		/*
		 * If there is an alpha value > 0, do the work, otherwise skip
		 */
		if (vram[s+0]) {
			uint8_t a = ((alpha * vram[s+0]) + vram[s+0]) >> 8;
			uint8_t r = ((gamma_red * vram[s+1]) + vram[s+1]) >> 8;
			uint8_t g = ((gamma_green * vram[s+2]) + vram[s+2]) >> 8;
			uint8_t b = ((gamma_blue * vram[s+3]) + vram[s+3]) >> 8;

			vram[d+1] = ((a * (r - vram[d+1])) + r + (vram[d+1] << 8)) >> 8;
			vram[d+2] = ((a * (g - vram[d+2])) + g + (vram[d+2] << 8)) >> 8;
			vram[d+3] = ((a * (b - vram[d+3])) + b + (vram[d+3] << 8)) >> 8;
			vram[d+0] = 0xff;	// result is always alpha full
			//vram[d+0] = (65536 - ((256 - vram[s+0]) * (256 - vram[d+0]))) >> 8;
		}
	}

	/*
	 * All return number of pixels changed
	 */
	uint32_t blit(const uint8_t s, const uint8_t d);
	uint32_t tile_blit(const uint8_t s, const uint8_t d, const uint8_t _ts);
	uint32_t clear_surface(const uint8_t dest);
	uint32_t pset(int16_t x0, int16_t y0, uint8_t d);
	uint32_t line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d);
	uint32_t rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d);
	uint32_t solid_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d);

	void set_pixel_saldo(uint32_t s) { pixel_saldo = s; }
	uint32_t get_pixel_saldo() { return pixel_saldo; }

	uint8_t *vram;
};

#endif
