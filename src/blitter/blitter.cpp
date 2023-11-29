#include "blitter.hpp"
#include "common.hpp"
#include <cstdio>

blitter_ic::blitter_ic()
{
	vram = new uint8_t[VRAM_SIZE];
	
	font_4x6 = new uint8_t[8192];
	for (int i = 0; i < 8192; i++) font_4x6[i] = 0;
	init_font_4x6();
	
	/* framebuffer is a surface as well */
	framebuffer.base = framebuffer_bank << 16;
	framebuffer.x = 0;
	framebuffer.y = 0;
	framebuffer.w = MAX_PIXELS_PER_SCANLINE;
	framebuffer.h = MAX_SCANLINES;
	framebuffer.bg_col = 0b000000101;
	
	/*
	 * available font
	 */
	font.flags_0 = 0b01000000;
	font.flags_1 = 0b00000000;
	font.fg_col = 0b00111000;
	font.keycolor = C64_BLUE;
	font.w = 4;
	font.h = 6;
	
	// for (int i=0; i< (256*4*6); i++) {
	// 	vram[font.base + i] = font_4x6[i];
	// }
	
	tile_surface text_buffer;
	text_buffer.columns = 80;
	text_buffer.rows = 4;
	text_buffer.x = 0;
	text_buffer.y = 0;
	text_buffer.base = 0x10000;
	for (int i=0; i<256; i++) {
		vram[text_buffer.base + i] = i;
	}
	vram[text_buffer.base + 0x14] = 0x45;	// E
	vram[text_buffer.base + 0x15] = 0x6c;	// l
	vram[text_buffer.base + 0x16] = 0x6d;	// m
	vram[text_buffer.base + 0x17] = 0x65;	// e
	vram[text_buffer.base + 0x18] = 0x72;	// r
	
	//surface turn_text;
	turn_text.index = 0;
	turn_text.flags_0 = 0b00000000;
	turn_text.flags_1 = 0b00010000;
	turn_text.base= 0x30000;
	turn_text.w = 320;
	turn_text.h = 24;
	turn_text.x = 50;
	turn_text.y = 65;
	tile_blit(&text_buffer, &font, &turn_text);
	
	for (int i=0; i<184; i++) {
		vram[0x300+i] = bruce_data[i];
	}
	bruce.index = 0;
	bruce.base = 0x300;
	bruce.keycolor = 0x01;
	bruce.flags_0 = 0b00000001;
	bruce.flags_1 = 0b00000001;
	bruce.w = 8;
	bruce.h = 23;
	bruce.x = 170;
	bruce.y = 42;
	
	for (int i=0; i< 169; i++) {
		vram[0x400+i] = punch_data[i];
	}
	punch.base = 0x400;
	punch.keycolor = 0x01;
	punch.flags_0 = 0b00000001;
	punch.flags_1 = 0b00000000;
	punch.w = 13;
	punch.h = 13;
	punch.x = 190;
	punch.y = 52;
}

blitter_ic::~blitter_ic()
{
	delete [] font_4x6;
	delete [] vram;
}

/*
 * - Check with keycolor
 */
uint32_t blitter_ic::blit(const surface *src, surface *dest)
{
	uint32_t pixelcount = 0;
	
	auto min = [](int16_t a, int16_t b) { return a < b ? a : b; };
	auto max = [](int16_t a, int16_t b) { return a > b ? a : b; };
	auto swap = [](int16_t &a, int16_t &b) { int16_t c = a; a = b; b = c; };
	
	uint8_t dw = (src->flags_1 & FLAGS1_DBLWIDTH) ? 1 : 0;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) ? 1 : 0;
	
	int16_t startx, endx, starty, endy;
	
	/*
	 * Following values are coordinates in the src rectangle
	 */
	if (!(src->flags_1 & FLAGS1_XY_FLIP)) {
		startx = max(0, -src->x);
		endx = min(src->w << dw, -src->x + dest->w);
		starty = max(0, -src->y);
		endy = min(src->h << dh, -src->y + dest->h);
	} else {
		startx = max(0, -src->y);
		endx = min(src->w << dw, -src->y + dest->h);
		starty = max(0, -src->x);
		endy = min(src->h << dh, -src->x + dest->w);
	}
	
	if (src->flags_1 & FLAGS1_HOR_FLIP) {
		int16_t temp_value = startx;
		startx = (src->w << dw) - endx;
		endx   = (src->w << dw) - temp_value;
	}
	
	if (src->flags_1 & FLAGS1_VER_FLIP) {
		int16_t temp_value = starty;
		starty = (src->h << dh) - endy;
		endy   = (src->h << dh) - temp_value;
	}

	uint32_t offset = (src->index * src->w * src->h);
	
	int16_t dest_x;
	int16_t dest_y;

	for (int y = starty; y < endy; y++) {
		for (int x = startx; x < endx; x++) {
			uint8_t px{0};
			
			switch ((src->flags_0 & 0b11000000) >> 6) {
				case 0b00:
					px = vram[src->base + (offset + (x >> dw) + (src->w * (y >> dh))) & VRAM_SIZE_MASK];
					break;
				case 0b01:
				case 0b10:
				case 0b11:
					px = font_4x6[(offset + (x >> dw) + (src->w * (y >> dh))) & 0x1fff];
					break;
			}
			
			if (src->flags_1 & FLAGS1_HOR_FLIP) dest_x = (src->w << dw) - 1 - x; else dest_x = x;
			if (src->flags_1 & FLAGS1_VER_FLIP) dest_y = (src->h << dh) - 1 - y; else dest_y = y;
			if (src->flags_1 & FLAGS1_XY_FLIP) swap(dest_x, dest_y);
			
			/*
			 * TODO: Can this be sped up?
			 */
			switch (src->flags_0 & 0b111) {
				case 0b000:
				case 0b100:
					vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					break;
				case 0b010:
				case 0b110:
					vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->fg_col;
					break;
				case 0b001:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					}
					break;
				case 0b011:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->fg_col;
					}
					break;
				case 0b101:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					} else {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->bg_col;
					}
					break;
				case 0b111:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->fg_col;
					} else {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->bg_col;
					}
					break;
			}
			pixelcount++;
		}
	}
	return pixelcount;
}

uint32_t blitter_ic::tile_blit(const tile_surface *ts, const surface *src, surface *dst)
{
	uint32_t pixelcount = 0;
	
	surface source = *src;
	
	uint8_t dw = (src->flags_1 & FLAGS1_DBLWIDTH) ? 1 : 0;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) ? 1 : 0;
	
	source.x = ts->x;
	source.y = ts->y;
	uint32_t tile = ts->base;
	uint32_t fg_color = ts->base + (ts->columns * ts->rows);
	uint32_t bg_color = ts->base + (2 * ts->columns * ts->rows);
	
	for (int y = 0; y < ts->rows; y++) {
		for (int x = 0; x < ts->columns; x++) {
			source.index = vram[tile++ & VRAM_SIZE_MASK];
			source.fg_col = vram[fg_color++ & VRAM_SIZE_MASK];
			source.bg_col = vram[bg_color++ & VRAM_SIZE_MASK];
			pixelcount += blit(&source, dst);
			source.x += (source.w << dw);
		}
		source.x = ts->x;
		source.y += (source.h << dh);
	}
	
	return pixelcount;
}

uint32_t blitter_ic::clear_surface(const surface *s)
{
	uint32_t pixels = s->w * s->h;
	for (uint32_t i=0; i < pixels; i++) {
		vram[(s->base + i) & VRAM_SIZE_MASK] = s->bg_col;
	}
	return pixels;
}

/*
 * https://hackaday.io/project/6309-vga-graphics-over-spi-and-serial-vgatonic/log/20759-a-tiny-4x6-pixel-font-that-will-fit-on-almost-any-microcontroller-license-mit
 */
const uint8_t font4x6_orig [96][2] = {
	{  0x00  ,  0x00  },   /*' '*/
	{  0x49  ,  0x08  },   /*'!'*/
	{  0xb4  ,  0x00  },   /*'"'*/
	{  0xbe  ,  0xf6  },   /*'#'*/
	{  0x7b  ,  0x7a  },   /*'$'*/
	{  0xa5  ,  0x94  },   /*'%'*/
	{  0x55  ,  0xb8  },   /*'&'*/
	{  0x48  ,  0x00  },   /*'''*/
	{  0x29  ,  0x44  },   /*'('*/
	{  0x44  ,  0x2a  },   /*')'*/
	{  0x15  ,  0xa0  },   /*'*'*/
	{  0x0b  ,  0x42  },   /*'+'*/
	{  0x00  ,  0x51  },   /*','*/ /* 1 pixel lower than original version */
	{  0x03  ,  0x02  },   /*'-'*/
	{  0x00  ,  0x08  },   /*'.'*/
	{  0x25  ,  0x90  },   /*'/'*/
	{  0x76  ,  0xba  },   /*'0'*/
	{  0x59  ,  0x5c  },   /*'1'*/
	{  0xc5  ,  0x9e  },   /*'2'*/
	{  0xc5  ,  0x38  },   /*'3'*/
	{  0x92  ,  0xe6  },   /*'4'*/
	{  0xf3  ,  0x3a  },   /*'5'*/
	{  0x73  ,  0xba  },   /*'6'*/
	{  0xe5  ,  0x90  },   /*'7'*/
	{  0x77  ,  0xba  },   /*'8'*/
	{  0x77  ,  0x3a  },   /*'9'*/
	{  0x08  ,  0x40  },   /*':'*/
	{  0x08  ,  0x50  },   /*';'*/
	{  0x2a  ,  0x44  },   /*'<'*/
	{  0x1c  ,  0xe0  },   /*'='*/
	{  0x88  ,  0x52  },   /*'>'*/
	{  0xe5  ,  0x08  },   /*'?'*/
	{  0x56  ,  0x8e  },   /*'@'*/
	{  0x77  ,  0xb6  },   /*'A'*/
	{  0x77  ,  0xb8  },   /*'B'*/
	{  0x72  ,  0x8c  },   /*'C'*/
	{  0xd6  ,  0xba  },   /*'D'*/
	{  0x73  ,  0x9e  },   /*'E'*/
	{  0x73  ,  0x92  },   /*'F'*/
	{  0x72  ,  0xae  },   /*'G'*/
	{  0xb7  ,  0xb6  },   /*'H'*/
	{  0xe9  ,  0x5c  },   /*'I'*/
	{  0x64  ,  0xaa  },   /*'J'*/
	{  0xb7  ,  0xb4  },   /*'K'*/
	{  0x92  ,  0x9c  },   /*'L'*/
	{  0xbe  ,  0xb6  },   /*'M'*/
	{  0xd6  ,  0xb6  },   /*'N'*/
	{  0x56  ,  0xaa  },   /*'O'*/
	{  0xd7  ,  0x92  },   /*'P'*/
	{  0x76  ,  0xee  },   /*'Q'*/
	{  0x77  ,  0xb4  },   /*'R'*/
	{  0x71  ,  0x38  },   /*'S'*/
	{  0xe9  ,  0x48  },   /*'T'*/
	{  0xb6  ,  0xae  },   /*'U'*/
	{  0xb6  ,  0xaa  },   /*'V'*/
	{  0xb6  ,  0xf6  },   /*'W'*/
	{  0xb5  ,  0xb4  },   /*'X'*/
	{  0xb5  ,  0x48  },   /*'Y'*/
	{  0xe5  ,  0x9c  },   /*'Z'*/
	{  0x69  ,  0x4c  },   /*'['*/
	{  0x91  ,  0x24  },   /*'\'*/
	{  0x64  ,  0x2e  },   /*']'*/
	{  0x54  ,  0x00  },   /*'^'*/
	{  0x00  ,  0x1c  },   /*'_'*/
	{  0x44  ,  0x00  },   /*'`'*/
	{  0x0e  ,  0xae  },   /*'a'*/
	{  0x9a  ,  0xba  },   /*'b'*/
	{  0x0e  ,  0x8c  },   /*'c'*/
	{  0x2e  ,  0xae  },   /*'d'*/
	{  0x0e  ,  0xce  },   /*'e'*/
	{  0x56  ,  0xd0  },   /*'f'*/
	{  0x55  ,  0x3B  },   /*'g'*/
	{  0x93  ,  0xb4  },   /*'h'*/
	{  0x41  ,  0x48  },   /*'i'*/  // 010 000 010 010 010  01000001 01001000  41 44
	{  0x09  ,  0x51  },   /*'j'*/  // adjusted, missing dot, add later, repair
	{  0x97  ,  0xb4  },   /*'k'*/
	{  0x49  ,  0x44  },   /*'l'*/
	{  0x17  ,  0xb6  },   /*'m'*/
	{  0x1a  ,  0xb6  },   /*'n'*/
	{  0x0a  ,  0xaa  },   /*'o'*/
	{  0xd6  ,  0xd3  },   /*'p'*/
	{  0x76  ,  0x67  },   /*'q'*/
	{  0x17  ,  0x90  },   /*'r'*/
	{  0x0f  ,  0x38  },   /*'s'*/
	{  0x9a  ,  0x8c  },   /*'t'*/
	{  0x16  ,  0xae  },   /*'u'*/
	{  0x16  ,  0xba  },   /*'v'*/
	{  0x16  ,  0xf6  },   /*'w'*/
	{  0x15  ,  0xb4  },   /*'x'*/
	{  0xb5  ,  0x2b  },   /*'y'*/
	{  0x1c  ,  0x5e  },   /*'z'*/
	{  0x6b  ,  0x4c  },   /*'{'*/
	{  0x49  ,  0x48  },   /*'|'*/
	{  0xc9  ,  0x5a  },   /*'}'*/
	{  0x54  ,  0x00  },   /*'~'*/
	{  0x56  ,  0xe2  }    /*''*/
};

const uint8_t blocks[108] = {
	0b0000,
	0b0000,
	0b0000,
	0b0000,
	0b0000,
	0b0000,
	
	0b1100,
	0b1100,
	0b1100,
	0b0000,
	0b0000,
	0b0000,
	
	0b0011,
	0b0011,
	0b0011,
	0b0000,
	0b0000,
	0b0000,
	
	0b1111,
	0b1111,
	0b1111,
	0b0000,
	0b0000,
	0b0000,
	
	0b0000,
	0b0000,
	0b0000,
	0b1100,
	0b1100,
	0b1100,
	
	0b1100,
	0b1100,
	0b1100,
	0b1100,
	0b1100,
	0b1100,
	
	0b0011,
	0b0011,
	0b0011,
	0b1100,
	0b1100,
	0b1100,
	
	0b1111,
	0b1111,
	0b1111,
	0b1100,
	0b1100,
	0b1100,
	
	0b0000,
	0b0000,
	0b0000,
	0b0011,
	0b0011,
	0b0011,
	
	0b1100,
	0b1100,
	0b1100,
	0b0011,
	0b0011,
	0b0011,
	
	0b0011,
	0b0011,
	0b0011,
	0b0011,
	0b0011,
	0b0011,
	
	0b1111,
	0b1111,
	0b1111,
	0b0011,
	0b0011,
	0b0011,
	
	0b0000,
	0b0000,
	0b0000,
	0b1111,
	0b1111,
	0b1111,
	
	0b1100,
	0b1100,
	0b1100,
	0b1111,
	0b1111,
	0b1111,
	
	0b0011,
	0b0011,
	0b0011,
	0b1111,
	0b1111,
	0b1111,
	
	0b1111,
	0b1111,
	0b1111,
	0b1111,
	0b1111,
	0b1111,
	
	0b0110,
	0b0110,
	0b1110,
	0b1110,
	0b0000,
	0b0000,
	
	0b0110,
	0b0110,
	0b0111,
	0b0111,
	0b0000,
	0b0000
};

// Font retrieval function - ugly, but needed.
inline bool get_font_4x6_pixel(uint8_t data, uint8_t x, uint8_t y)
{
	if (data < 32) return false;

	const uint8_t index = (data-32);
	uint8_t pixel = 0;
	if ((font4x6_orig[index][1] & 1) == 1) y -= 1;
	if (y == 0) {
		pixel = (font4x6_orig[index][0]) >> 4;
	} else if (y == 1) {
		pixel = (font4x6_orig[index][0]) >> 1;
	} else if (y == 2) {
		// Split over 2 bytes
		pixel = ((font4x6_orig[index][0] & 0x03) << 2) | ((font4x6_orig[index][1]) & 0x02);
	} else if (y == 3) {
		pixel = (font4x6_orig[index][1]) >> 4;
	} else if (y == 4) {
		pixel = (font4x6_orig[index][1]) >> 1;
	}
	x &= 0b11;
	return (pixel & 0b1110) & (1 << (3 - x)) ? true : false;
}

void blitter_ic::init_font_4x6()
{
	for (int i=0; i<128; i++){
		for (int y=0; y<6; y++) {
			for (int x=0; x<4; x++) {
				if (get_font_4x6_pixel(i, x, y)) {
					font_4x6[(i * 4 * 6) + (y * 4) + x] = C64_LIGHTBLUE;
					font_4x6[((i + 128) * 4 * 6) + (y * 4) + x] = C64_BLUE;
				} else {
					font_4x6[(i * 4 * 6) + (y * 4) + x] = C64_BLUE;
					font_4x6[((i + 128) * 4 * 6) + (y * 4) + x] = C64_LIGHTBLUE;
				}
			}
		}
	}
	
	// correction for j
	font_4x6[(0x6a * 4 * 6) + 1] = C64_LIGHTBLUE;
	font_4x6[((0x6a + 0x80) * 4 * 6) + 1] = C64_BLUE;
	
	// 6 bytes / symbol * 18 symbols = 108
	for (int i=0; i<(108); i++) {
		for (int j=0; j<4; j++) {
			if (blocks[i] & (0b1<<(3-j))) {
				font_4x6[(i*4)+j] = C64_LIGHTBLUE;
				font_4x6[(128*4*6)+(i*4)+j] = C64_BLUE;
			} else {
				font_4x6[(i*4)+j] = C64_BLUE;
				font_4x6[(128*4*6)+(i*4)+j] = C64_LIGHTBLUE;
			}
		}
	}
}
