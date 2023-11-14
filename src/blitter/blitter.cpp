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
	blob.keycolor = 0xdd;
	blob.index = 1;
	blob.x = 2;
	blob.y = 85;
	blob.w = 4;
	blob.h = 6;
	
	for (uint16_t i=1024; i<1073; i++) {
		vram[i] = blob_data[i-1024];
	}
}

blitter_ic::~blitter_ic()
{
	delete [] vram;
}

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
