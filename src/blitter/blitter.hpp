#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

typedef struct {
	/*
	 * bit
	 *  0  alpha
	 *  1  index or fixed color
	 *  2
	 *  3
	 *  4  hor flip
	 *  5  ver flip
	 *  6  hor stretch
	 *  7  ver stretch
	 */
	uint8_t  flags1;
	uint8_t  flags2;
	uint8_t  keycolor;
	uint8_t  index;
	uint32_t base;
	int16_t  x;
	int16_t  y;
	uint16_t w;
	uint16_t h;
} surface;

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	surface screen;
	surface blob;
	
	/*
	 * returns number of pixels blitted
	 */
	uint32_t blit(surface *src, surface *dst);
	
	uint32_t rectangle();	// use to wipe surface?
	uint32_t line();
	uint32_t circle();
	
	
	//uint16_t palette[256];
	uint8_t *vram;
private:
	const uint8_t blob_data[48] = {
		0xae,0x18,0x18,0x18,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x18,0x00,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x18,0x18,
		0x00,0x00,0x00,0x00,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x00,0x00,
		0x00,0x18,0x18,0x18,
		0x00,0x00,0x00,0x00
	};
	
	uint8_t *tiny_4x6_pixel_font;
	void init_tiny_4x6_pixel_font();
};

#endif
