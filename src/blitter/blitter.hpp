#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

#define FLAGS0_ROMFONT		0b01000000

#define	FLAGS1_DBLWIDTH		0b00000001
#define FLAGS1_DBLHEIGHT	0b00000010
#define FLAGS1_HOR_FLIP		0b00010000
#define FLAGS1_VER_FLIP		0b00100000
#define	FLAGS1_XY_FLIP		0b01000000

typedef struct {
	/*
	 * Properties related to flags_0 (as encoded inside machine)
	 * Color related
	 *
	 * 7 6 5 4 3 2 1 0
	 * | |       | | |
	 * | |       | | +-- Keycolor (0 = off, 1 = on)
	 * | |       | +---- Use foreground color (0 = off, 1 = on)
	 * | |       +------ Use background color (0 = off, 1 = on)
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
	
	surface font;
	surface framebuffer;
	surface turn_text;
	surface bruce;
	surface punch;
	
	/*
	 * all return number of pixels changed
	 */
	uint32_t blit(const surface *src, surface *dst);
	uint32_t tile_blit(const surface *src, surface *dst, const tile_surface *ts);
	uint32_t clear_surface(const surface *s);
	
//	uint32_t rectangle();
//	uint32_t line();
//	uint32_t circle();
	
	uint8_t *vram;
	
	uint8_t framebuffer_bank = 0x0e;
private:
	uint8_t *font_4x6;
	void init_font_4x6();
	
	const uint8_t bruce_data[23*8] = {
		0x01,0x01,0x00,0x00,0x01,0x01,0x01,0x01,
		0x01,0x00,0x00,0xf2,0x01,0x01,0x01,0x01,
		0x01,0x00,0xf2,0xf2,0x00,0x01,0x01,0x01,
		0x01,0x00,0xf2,0x00,0xf2,0x01,0x01,0x01,
		0x01,0x00,0xf2,0xf2,0xf2,0x01,0x01,0x01,
		0x01,0x01,0xf2,0xf2,0xf2,0x01,0x01,0x01,
		0x01,0x01,0xf2,0xf2,0x01,0x01,0x01,0x01,
		0x01,0xf2,0xf2,0xf2,0x01,0x01,0x01,0x01,
		0x01,0xf2,0xf2,0xf2,0xf2,0x01,0x01,0x01,
		0x01,0xf2,0xf2,0x00,0xf2,0xf2,0x01,0x00,
		0x01,0xf2,0x00,0x00,0xf2,0xf2,0xf2,0x00,
		0x01,0x01,0xf2,0xf2,0xf2,0x01,0x01,0x01,
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,
		0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x01,
		0x01,0x01,0x00,0x01,0x00,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x01,0x00,0x01,0x01,0x00,0x01,0x01,
		0x01,0x00,0x00,0x01,0x01,0x00,0x00,0x01,
		0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00
	};
	
	const uint8_t punch_data[13*13] = {
		0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x08, 0x08, 0x5e, 0x5e, 0x5e, 0x08, 0x08, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x08, 0x5e, 0x5e, 0xce, 0xce, 0xce, 0x5e, 0x5e, 0x08, 0x01, 0x01,
		0x01, 0x08, 0x5e, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 0x5e, 0x08, 0x01,
		0x01, 0x08, 0x5e, 0xce, 0xce, 0xce, 0x00, 0xce, 0xce, 0xce, 0x5e, 0x08, 0x01,
		0x08, 0x5e, 0xce, 0xce, 0x00, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 0x5e, 0x08,
		0x08, 0x5e, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 0xce, 0x00, 0xce, 0x5e, 0x08,
		0x08, 0x5e, 0xce, 0xce, 0xce, 0xce, 0xce, 0x00, 0xce, 0xce, 0xce, 0x5e, 0x08,
		0x01, 0x08, 0x5e, 0xce, 0x00, 0xce, 0xce, 0xce, 0xce, 0xce, 0x5e, 0x08, 0x01,
		0x01, 0x08, 0x5e, 0xce, 0xce, 0xce, 0x00, 0xce, 0xce, 0xce, 0x5e, 0x08, 0x01,
		0x01, 0x01, 0x08, 0x5e, 0x5e, 0xce, 0xce, 0xce, 0x5e, 0x5e, 0x08, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x08, 0x08, 0x5e, 0x5e, 0x5e, 0x08, 0x08, 0x01, 0x01, 0x01,
		0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01
	};
};

#endif
