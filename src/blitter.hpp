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

#define	FLAGS1_DBLWIDTH		0b00000011
#define FLAGS1_DBLHEIGHT	0b00001100
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
	 *     | |     | |
	 *     | |     | |
	 *     | |     | +-- Use background color (0 = off, 1 = on)
	 *     | |     +---- Use foreground color (0 = off, 1 = on)
	 *     +-+---------- Bits per pixel (0b00 = 1, 0b01 = 2, 0b10 = 4, 0b11 = 8)
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
	 *   | | | | | | |
	 *   | | | | | +-+-- Width (00 = 1x, 01 = 2x, 10 = 3x, 11 = 4x)
	 *   | | | +-+------ Height (00 = 1x, 01 = 2x, 10 = 3x, 11 = 4x)
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
	bool little_endian;

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

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	uint8_t io_surfaces_read8(uint16_t address);
	void io_surfaces_write8(uint16_t address, uint8_t value);

	uint8_t io_color_indices_read8(uint16_t address);
	void io_color_indices_write8(uint16_t address, uint8_t value);

	uint8_t io_palette_read8(uint16_t address) {
		switch (address & 0b11) {
			case 0b00: return (palette[(address & 0x3ff) >> 2] & 0xff000000) >> 24;
			case 0b01: return (palette[(address & 0x3ff) >> 2] & 0x00ff0000) >> 16;
			case 0b10: return (palette[(address & 0x3ff) >> 2] & 0x0000ff00) >>  8;
			case 0b11: return (palette[(address & 0x3ff) >> 2] & 0x000000ff) >>  0;
			default: return 0x00;
		}
	}

	void io_palette_write8(uint16_t address, uint8_t value) {
		switch (address & 0b11) {
			case 0b00: palette[(address & 0x3ff) >> 2] = (palette[(address & 0x3ff) >> 2] & 0x00ffffff) | (value << 24); break;
			case 0b01: palette[(address & 0x3ff) >> 2] = (palette[(address & 0x3ff) >> 2] & 0xff00ffff) | (value << 16); break;
			case 0b10: palette[(address & 0x3ff) >> 2] = (palette[(address & 0x3ff) >> 2] & 0xffff00ff) | (value <<  8); break;
			case 0b11: palette[(address & 0x3ff) >> 2] = (palette[(address & 0x3ff) >> 2] & 0xffffff00) | (value <<  0); break;
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

	void update_framebuffer();

	uint8_t *vram;
	uint32_t vram_peek{0};	// base address for vram peek page

	uint32_t *framebuffer;

	uint32_t palette[256] = {
		0xff000000,
		0xff000000,
		0xff000000,
		0xff000000,
		0xff000011,
		0xff000022,
		0xff000044,
		0xff000055,
		0xff000033,
		0xff000055,
		0xff000088,
		0xff0000aa,
		0xff000055,
		0xff000088,
		0xff0000cc,
		0xff0000ff,
		0xff001100,
		0xff002200,
		0xff004400,
		0xff005500,
		0xff001111,
		0xff002222,
		0xff004444,
		0xff005555,
		0xff001133,
		0xff002255,
		0xff004488,
		0xff0055aa,
		0xff001155,
		0xff002288,
		0xff0044cc,
		0xff0055ff,
		0xff003300,
		0xff005500,
		0xff008800,
		0xff00aa00,
		0xff003311,
		0xff005522,
		0xff008844,
		0xff00aa55,
		0xff003333,
		0xff005555,
		0xff008888,
		0xff00aaaa,
		0xff003355,
		0xff005588,
		0xff0088cc,
		0xff00aaff,
		0xff005500,
		0xff008800,
		0xff00cc00,
		0xff00ff00,
		0xff005511,
		0xff008822,
		0xff00cc44,
		0xff00ff55,
		0xff005533,
		0xff008855,
		0xff00cc88,
		0xff00ffaa,
		0xff005555,
		0xff008888,
		0xff00cccc,
		0xff00ffff,
		0xff110000,
		0xff220000,
		0xff440000,
		0xff550000,
		0xff110011,
		0xff220022,
		0xff440044,
		0xff550055,
		0xff110033,
		0xff220055,
		0xff440088,
		0xff5500aa,
		0xff110055,
		0xff220088,
		0xff4400cc,
		0xff5500ff,
		0xff111100,
		0xff222200,
		0xff444400,
		0xff555500,
		0xff111111,
		0xff222222,
		0xff444444,
		0xff555555,
		0xff111133,
		0xff222255,
		0xff444488,
		0xff5555aa,
		0xff111155,
		0xff222288,
		0xff4444cc,
		0xff5555ff,
		0xff113300,
		0xff225500,
		0xff448800,
		0xff55aa00,
		0xff113311,
		0xff225522,
		0xff448844,
		0xff55aa55,
		0xff113333,
		0xff225555,
		0xff448888,
		0xff55aaaa,
		0xff113355,
		0xff225588,
		0xff4488cc,
		0xff55aaff,
		0xff115500,
		0xff228800,
		0xff44cc00,
		0xff55ff00,
		0xff115511,
		0xff228822,
		0xff44cc44,
		0xff55ff55,
		0xff115533,
		0xff228855,
		0xff44cc88,
		0xff55ffaa,
		0xff115555,
		0xff228888,
		0xff44cccc,
		0xff55ffff,
		0xff330000,
		0xff550000,
		0xff880000,
		0xffaa0000,
		0xff330011,
		0xff550022,
		0xff880044,
		0xffaa0055,
		0xff330033,
		0xff550055,
		0xff880088,
		0xffaa00aa,
		0xff330055,
		0xff550088,
		0xff8800cc,
		0xffaa00ff,
		0xff331100,
		0xff552200,
		0xff884400,
		0xffaa5500,
		0xff331111,
		0xff552222,
		0xff884444,
		0xffaa5555,
		0xff331133,
		0xff552255,
		0xff884488,
		0xffaa55aa,
		0xff331155,
		0xff552288,
		0xff8844cc,
		0xffaa55ff,
		0xff333300,
		0xff555500,
		0xff888800,
		0xffaaaa00,
		0xff333311,
		0xff555522,
		0xff888844,
		0xffaaaa55,
		0xff333333,
		0xff555555,
		0xff888888,
		0xffaaaaaa,
		0xff333355,
		0xff555588,
		0xff8888cc,
		0xffaaaaff,
		0xff335500,
		0xff558800,
		0xff88cc00,
		0xffaaff00,
		0xff335511,
		0xff558822,
		0xff88cc44,
		0xffaaff55,
		0xff335533,
		0xff558855,
		0xff88cc88,
		0xffaaffaa,
		0xff335555,
		0xff558888,
		0xff88cccc,
		0xffaaffff,
		0xff550000,
		0xff880000,
		0xffcc0000,
		0xffff0000,
		0xff550011,
		0xff880022,
		0xffcc0044,
		0xffff0055,
		0xff550033,
		0xff880055,
		0xffcc0088,
		0xffff00aa,
		0xff550055,
		0xff880088,
		0xffcc00cc,
		0xffff00ff,
		0xff551100,
		0xff882200,
		0xffcc4400,
		0xffff5500,
		0xff551111,
		0xff882222,
		0xffcc4444,
		0xffff5555,
		0xff551133,
		0xff882255,
		0xffcc4488,
		0xffff55aa,
		0xff551155,
		0xff882288,
		0xffcc44cc,
		0xffff55ff,
		0xff553300,
		0xff885500,
		0xffcc8800,
		0xffffaa00,
		0xff553311,
		0xff885522,
		0xffcc8844,
		0xffffaa55,
		0xff553333,
		0xff885555,
		0xffcc8888,
		0xffffaaaa,
		0xff553355,
		0xff885588,
		0xffcc88cc,
		0xffffaaff,
		0xff555500,
		0xff888800,
		0xffcccc00,
		0xffffff00,
		0xff555511,
		0xff888822,
		0xffcccc44,
		0xffffff55,
		0xff555533,
		0xff888855,
		0xffcccc88,
		0xffffffaa,
		0xff555555,
		0xff888888,
		0xffcccccc,
		0xffffffff
	};
};

#endif

//	/*
//	 * std RGB332 palette
//	 */
//	for (int i = 0; i < 256; i++) {
//		uint16_t r = (i & 0b11100000) >> 5;
//		uint16_t g = (i & 0b00011100) >> 2;
//		uint16_t b = (i & 0b00000011) >> 0;
//		r = (15 * r) / 7;
//		g = (15 * g) / 7;
//		b = (15 * b) / 3;
//		//printf("No:%03i r:%02i g:%02i b:%02i\n", i, r, g, b);
//		palette[i] = 0b1111000000000000 | (r << 8) | (g << 4) | (b << 0);
//	}

	// /*
	//  * A palette using RRGGBBII system. R, G and B use two bits and have
	//  * 4 levels each (0.00, 0.33, 0.66 and 1.00 of max). On top of that,
	//  * the intensity level (II) is shared between all channels.
	//  *
	//  * Final color levels are RR * II, GG * II and BB * II.
	//  *
	//  * II is not linear, see below. This system results in a nice palette
	//  * with many dark shades as well to choose from (compared to RGB332).
	//  *
	//  * Inspired by: https://www.bigmessowires.com/2008/07/04/video-palette-setup/
	//  */
	// for (int i = 0; i < 256; i++) {
	// 	uint32_t r = (i & 0b11000000) >> 6;
	// 	uint32_t g = (i & 0b00110000) >> 4;
	// 	uint32_t b = (i & 0b00001100) >> 2;
	// 	uint32_t s = (i & 0b00000011) >> 0;
	// 	uint32_t factor = 0;

	// 	switch (s) {
	// 		case 0b00: factor =  5; break;
	// 		case 0b01: factor =  8; break;
	// 		case 0b10: factor = 12; break;
	// 		case 0b11: factor = 15; break;
	// 	}

	// 	// TODO: Unfortunately, rounding here is key to the colors that
	// 	// can be seen. Maybe optimize this somehow?
	// 	r = 17 * ((factor * r) / 3);
	// 	g = 17 * ((factor * g) / 3);
	// 	b = 17 * ((factor * b) / 3);

	// 	palette[i] = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
	// }
