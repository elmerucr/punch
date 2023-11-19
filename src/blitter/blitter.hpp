#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

#define FRAMEBUFFER_BANK	0x00

#define	F1_DBLWIDTH	0b00000001
#define F1_DBLHEIGHT	0b00000010
#define F1_HOR_FLIP	0b00010000
#define F1-VER_FLIP	0b00100000
#define	F1_XY_FLIP	0b01000000

typedef struct {
	/*
	 * flags0
	 *
	 * bit
	 *  0  keycolor
	 *  1  fg
	 *  2  bg
	 *  3  -
	 *  4  -
	 *  5  -
	 *  6  -
	 *  7  -
	 */
	uint8_t  flags_0;
	
	/*
	 * Properties related to flags_1 (as encoded inside machine)
	 * Size, flips and rotations
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
	uint8_t  flags_1;
	
	uint8_t  keycolor;
	uint8_t  fgc;
	uint8_t  bgc;
	
	uint8_t  index;
	uint32_t base;
	int16_t  x;
	int16_t  y;
	uint16_t w;
	uint16_t h;
} surface;

typedef struct {
	uint8_t columns;
	uint8_t rows;
	uint32_t base;
	int16_t x;
	int16_t y;
} tile_surface;

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	surface screen;
	surface blob;
	
	/*
	 * both return number of pixels blitted
	 */
	uint32_t blit(const surface *src, surface *dst);
	uint32_t tile_blit(const surface *src, surface *dst, const tile_surface *ts);
	
	
	uint32_t rectangle();	// use to wipe surface?
	uint32_t line();
	uint32_t circle();
	
	
	//uint16_t palette[256];
	uint8_t *vram;
private:	
	uint8_t *tiny_4x6_pixel_font;
	void init_tiny_4x6_pixel_font();
	
	const uint8_t sprite[112] = {
		0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,
		0x01,0x00,0xfc,0xfc,0xfc,0xfc,0x00,0x01,
		0x00,0xfc,0x00,0xfc,0xfc,0x00,0xfc,0x00,
		0x00,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0x00,
		0x00,0xfc,0x00,0xfc,0xfc,0x00,0xfc,0x00,
		0x00,0xfc,0xfc,0x00,0x00,0xfc,0xfc,0x00,
		0x01,0x00,0xfc,0xfc,0xfc,0xfc,0x00,0x01,
		0x01,0x01,0x00,0xfc,0xfc,0x00,0x01,0x01,
		0x01,0x01,0x00,0xfc,0xfc,0x00,0x01,0x01,
		0x01,0x00,0xfc,0xfc,0xfc,0xfc,0x00,0x01,
		0x00,0xfc,0xfc,0xfc,0xfc,0xfc,0xfc,0x00,
		0x00,0x00,0x00,0x00,0x00,0xfc,0xfc,0x00,
		0x01,0x01,0x01,0x01,0x00,0xfc,0xfc,0x00,
		0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x01
	};
};

#endif
