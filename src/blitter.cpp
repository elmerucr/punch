/*
 * blitter.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "blitter.hpp"
#include "common.hpp"
#include <cstdio>

blitter_ic::blitter_ic()
{
	vram = new uint8_t[VRAM_SIZE];
	
	for (int i = 0; i < VRAM_SIZE; i++) {
		vram[i] = (i & 0x40) ? 0xff : 0x00;
	}
	
	framebuffer = new uint16_t[PIXELS];
	
	palette = new uint16_t[256];
	
//	// std RGB332 palette
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
	
	// different palette
	for (int i = 0; i < 256; i++) {
		uint16_t r = (i & 0b11000000) >> 6;
		uint16_t g = (i & 0b00110000) >> 4;
		uint16_t b = (i & 0b00001100) >> 2;
		uint16_t s = (i & 0b00000011) >> 0;
		uint16_t factor = 0;
		switch (s) {
			case 0b00: factor = 6; break;
			case 0b01: factor = 11; break;
			case 0b10: factor = 14; break;
			case 0b11: factor = 15; break;
		}
		r = factor * (r) / 3;
		g = factor * (g) / 3;
		b = factor * (b) / 3;
//		r = (r << 2) | s;
//		g = (g << 2) | s;
//		b = (b << 2) | s;
		//printf("No:%03i r:%02i g:%02i b:%02i\n", i, r, g, b);
		palette[i] = 0b1111000000000000 | (r << 8) | (g << 4) | (b << 0);
	}

	font_4x6 = new uint8_t[8192];
	for (int i = 0; i < 8192; i++) font_4x6[i] = 0;
	init_font_4x6();
	
	/*
	 * available font
	 */
	font.flags_0 = 0b01000000;
	font.flags_1 = 0b00000000;
	font.fg_col = 0b00111000;
	font.keycolor = C64_BLUE;
	font.w = 4;
	font.h = 6;
	
	tile_surface_t text_buffer;
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
}

blitter_ic::~blitter_ic()
{
	delete [] font_4x6;
	delete [] palette;
	delete [] framebuffer;
	delete [] vram;
}

/*
 * - Check with keycolor
 */
uint32_t blitter_ic::blit(const surface_t *src, surface_t *dest)
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
			
			/*
			 * Pixel selector from vram or font
			 */
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
			 * Color selection
			 * TODO: Can this be sped up?
			 */
			switch (src->flags_0 & 0b1101) {
				case 0b0000:
				case 0b1000:
					vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					break;
				case 0b0100:
				case 0b1100:
					vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->fg_col;
					break;
				case 0b0001:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					}
					break;
				case 0b0101:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->fg_col;
					}
					break;
				case 0b1001:
					if (px != src->keycolor) {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
					} else {
						vram[(dest->base + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->bg_col;
					}
					break;
				case 0b1101:
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

uint32_t blitter_ic::blit(const uint8_t s, const uint8_t d)
{
	return blit(&surface[s], &surface[d]);
}

uint32_t blitter_ic::tile_blit(const tile_surface_t *ts, const surface_t *src, surface_t *dst)
{
	uint32_t pixelcount = 0;
	
	surface_t source = *src;
	
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

uint32_t blitter_ic::clear_surface(const surface_t *s)
{
	uint32_t pixels = s->w * s->h;
	for (uint32_t i=0; i < pixels; i++) {
		vram[(s->base + i) & VRAM_SIZE_MASK] = s->bg_col;
	}
	return pixels;
}

uint32_t blitter_ic::clear_surface(const uint8_t s)
{
	return clear_surface(&surface[s & 0x07]);
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
	{  0x75  ,  0x3B  },   /*'g'*/  // 011 101 01 001 110 1
	{  0x9a  ,  0xb6  },   /*'h'*/  // 10011010 10110110   9a b6
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
	0b0000,	// 0
	0b0000,
	0b0000,
	0b0000,
	0b0000,
	0b0000,
	
	0b1100, // 1
	0b1100,
	0b1100,
	0b0000,
	0b0000,
	0b0000,
	
	0b0011, // 2
	0b0011,
	0b0011,
	0b0000,
	0b0000,
	0b0000,
	
	0b1111, // 3
	0b1111,
	0b1111,
	0b0000,
	0b0000,
	0b0000,
	
	0b0000, // 4
	0b0000,
	0b0000,
	0b1100,
	0b1100,
	0b1100,
	
	0b1100, // 5
	0b1100,
	0b1100,
	0b1100,
	0b1100,
	0b1100,
	
	0b0011, // 6
	0b0011,
	0b0011,
	0b1100,
	0b1100,
	0b1100,
	
	0b1111, // 7
	0b1111,
	0b1111,
	0b1100,
	0b1100,
	0b1100,
	
	0b0000, // 8
	0b0000,
	0b0000,
	0b0011,
	0b0011,
	0b0011,
	
	0b1100, // 9
	0b1100,
	0b1100,
	0b0011,
	0b0011,
	0b0011,
	
	0b0011, // a
	0b0011,
	0b0011,
	0b0011,
	0b0011,
	0b0011,
	
	0b1111, // b
	0b1111,
	0b1111,
	0b0011,
	0b0011,
	0b0011,
	
	0b0000, // c
	0b0000,
	0b0000,
	0b1111,
	0b1111,
	0b1111,
	
	0b1100, // d
	0b1100,
	0b1100,
	0b1111,
	0b1111,
	0b1111,
	
	0b0011, // e
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

uint8_t blitter_ic::io_read8(uint16_t address)
{
	if (address & 0x100) {
		switch (address & 0b00011111) {
			case 0b00000:
				return surface[(address & 0b11100000) >> 5].fg_col;
			case 0b00001:
				return surface[(address & 0b11100000) >> 5].bg_col;
			default:
				return 0x00;
		}
	} else {
		return 0x00;
	}
}

void blitter_ic::io_write8(uint16_t address, uint8_t value)
{
	if (address & 0x100) {
		switch (address & 0b00011111) {
			case 0b00000:
				surface[(address & 0b11100000) >> 5].fg_col = value;
				break;
			case 0b00001:
				surface[(address & 0b11100000) >> 5].bg_col = value;
				break;
			default:
				break;
		}
	} else {
		
	}
}

void blitter_ic::update_framebuffer()
{
	for (int i = 0; i < PIXELS; i++) {
		framebuffer[i] = palette[vram[FRAMEBUFFER + i]];
	}
}
