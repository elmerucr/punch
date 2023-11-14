#include "blitter.hpp"
#include "common.hpp"

blitter_ic::blitter_ic()
{
	vram = new uint8_t[VRAM_SIZE];
	
	screen.base = 0x10000;
	screen.x = 0;
	screen.y = 0;
	screen.w = MAX_PIXELS_PER_SCANLINE;
	screen.h = MAX_SCANLINES;
	
	blob.base = 0x0400;
	blob.keycolor = 0x00;
	blob.index = 0;
	blob.x = 2;
	blob.y = 85;
	blob.w = 4;
	blob.h = 6;
	
	for (uint16_t i=1024; i<1073; i++) {
		vram[i] = blob_data[i-1024];
	}
	
	/*
	 * RGB332 palette
	 * https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_graphics
	 */
	uint8_t base = 1;
	uint8_t rest = 15 - base;
	for (int i=0; i<256; i++) {
		uint16_t r = (base + (rest * ((i & 0b11100000) >> 5)) / 7) << 8;
		uint16_t g = (base + (rest * ((i & 0b00011100) >> 2)) / 7) << 4;
		uint16_t b = (base + (rest * ((i & 0b00000011) >> 0)) / 3);
		palette[i] = 0xf000 | r | g | b;
		vram[(MAX_PIXELS_PER_SCANLINE*16) + (((i & 0b11100000)>>5)*MAX_PIXELS_PER_SCANLINE) + (i & 0b11111)] = i;
	}
}

blitter_ic::~blitter_ic()
{
	delete [] vram;
}

/*
 * - Check with keycolor
 * - Do real value, or defined color(s)?
 * - How to do textblocks / map?
 * - (mis)use this to make blocks of one color?
 */
void blitter_ic::blit(rect *src, rect *dst)
{
	auto min = [](int16_t a, int16_t b) { return a < b ? a : b; };
	auto max = [](int16_t a, int16_t b) { return a > b ? a : b; };
	
	/*
	 * Following values are coordinates in the src rectangle
	 */
	int16_t startx = max(0, -src->x);
	int16_t endx = min(src->w, -src->x + dst->w);
	int16_t starty = max(0, -src->y);
	int16_t endy = min(src->h, -src->y + dst->h);
	
	for (int y=starty; y < endy; y++) {
		for (int x=startx; x < endx; x++) {
			uint8_t px = vram[(src->base + (src->index * src->w * src->h) + (y * src->w) + x) & VRAM_SIZE_MASK];
			if (px != src->keycolor) {
				vram[(dst->base + ((y + src->y) * dst->w) + x + src->x) & VRAM_SIZE_MASK] = px;
			}
		}
	}
}
