/*
 * blitter.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "blitter.hpp"
#include "common.hpp"
#include <cstdio>
#include <cmath>

blitter_ic::blitter_ic()
{
	vram = new uint8_t[VRAM_SIZE];

	framebuffer = new uint16_t[PIXELS];

	palette = new uint16_t[256];

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

	/*
	 * A palette using RRGGBBII system. R, G and B use two bits and have
	 * 4 levels each (0.00, 0.33, 0.66 and 1.00 of max). On top of that,
	 * the intensity level (II) is shared between all channels.
	 * Final color levels are RR * II, GG * II and BB * II.
	 * II is not linear, see below. This systems results in a nice palette
	 * with many dark shades as well to choose from (compared to RGB332).
	 *
	 * Inspired by: https://www.bigmessowires.com/2008/07/04/video-palette-setup/
	 */
	for (int i = 0; i < 256; i++) {
		uint16_t r = (i & 0b11000000) >> 6;
		uint16_t g = (i & 0b00110000) >> 4;
		uint16_t b = (i & 0b00001100) >> 2;
		uint16_t s = (i & 0b00000011) >> 0;
		uint16_t factor = 0;

		switch (s) {
			case 0b00: factor = 4; break;
			case 0b01: factor = 7; break;
			case 0b10: factor = 12; break;
			case 0b11: factor = 15; break;
		}

		r = (factor * r) / 3;
		g = (factor * g) / 3;
		b = (factor * b) / 3;

		//printf("\t0x00%02x%02x%02x,\n", 17 * r, 17 * g, 17 * b);

		palette[i] = 0b1111000000000000 | (r << 8) | (g << 4) | (b << 0);
	}
}

blitter_ic::~blitter_ic()
{
	delete [] palette;
	delete [] framebuffer;
	delete [] vram;
}

void blitter_ic::reset()
{
	for (int i = 0; i < VRAM_SIZE; i++) {
		vram[i] = (i & 0x40) ? 0xfc : 0x00;
	}
}

/*
 * Short indexed version. Returns number of pixels written.
 */
uint32_t blitter_ic::blit(const uint8_t s, const uint8_t d)
{
	return blit(&surface[s & 0b1111], &surface[d & 0b1111]);
}

/*
 * Returns number of pixels written.
 */
uint32_t blitter_ic::blit(const surface_t *src, surface_t *dest)
{
	uint32_t old_pixel_saldo = pixel_saldo;

	/*
	 * Convenience lambda functions
	 */
	auto min = [](int16_t a, int16_t b) { return a < b ? a : b; };
	auto max = [](int16_t a, int16_t b) { return a > b ? a : b; };
	auto swap = [](int16_t &a, int16_t &b) { int16_t c = a; a = b; b = c; };

	/*
	 * Calculate potential bitshifts for double width and height
	 */
	uint8_t dw = (src->flags_1 & FLAGS1_DBLWIDTH)  ? 1 : 0;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) ? 1 : 0;

	int16_t startx, endx, starty, endy;

	/*
	 * Following values are coordinates in the src rectangle
	 */
	if (!(src->flags_1 & FLAGS1_X_Y_FLIP)) {
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

	/*
	 * Pixel selector from vram or font + offset + mask selector
	 */
	uint8_t *memory{nullptr};
	uint32_t memory_mask{0};
	uint32_t start_address{0};
	uint32_t index = (src->index * src->w * src->h);

	switch (src->flags_2 & 0b00000111) {
		case 0b000:
		case 0b010:
		case 0b011:
		case 0b101:
		case 0b110:
		case 0b111:
			memory = vram;
			memory_mask = VRAM_SIZE_MASK;
			start_address = src->base_address;
			break;
		case 0b001:
			memory = font_4x6.data;
			memory_mask = 0x3ff;
			break;
		case 0b100:
			memory = font_cbm_8x8.data;
			memory_mask = 0x7ff;
			break;
	}

	uint8_t color_mode = (src->flags_0 & 0b00110000) >> 4;

	int16_t dest_x, dest_y;

	for (int y = starty; y < endy; y++) {
		for (int x = startx; x < endx; x++) {
			if (pixel_saldo) {
				/*
				 * Adjust placement locations if needed
				 */
				if (src->flags_1 & FLAGS1_HOR_FLIP) dest_x = (src->w << dw) - 1 - x; else dest_x = x;
				if (src->flags_1 & FLAGS1_VER_FLIP) dest_y = (src->h << dh) - 1 - y; else dest_y = y;
				if (src->flags_1 & FLAGS1_X_Y_FLIP) swap(dest_x, dest_y);

				/*
				 * Color selection
				 */
				uint32_t alt_index = index + (x >> dw) + (src->w * (y >> dh));	// index can't change during the for loops!
				uint8_t px = memory[(start_address + (alt_index / color_modes[color_mode].pixels_per_byte)) & memory_mask];

				// TODO: this looks like a mess
				px >>= color_modes[color_mode].bits_per_pixel * (color_modes[color_mode].pixels_per_byte - (alt_index % color_modes[color_mode].pixels_per_byte) - 1);
				px &= color_modes[color_mode].mask;
//				uint8_t px = memory[(base_address + index + (x >> dw) + (src->w * (y >> dh))) & memory_mask];

				bool px_present = px;

				if (color_modes[color_mode].color_lookup) px = src->color_indices[px];

				switch ((src->flags_0 & 0b011) | (px_present ? 0b100 : 0b000)) {
					case 0b000: // no pixel, bg off, fore off, do nothing
					case 0b010: // no pixel, bg off, fore on, do nothing
						break;
					case 0b001: // no pixel, bg on, fore off
					case 0b011: // no pixel, bg on, fore on
						vram[(dest->base_address + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->color_indices[0];
						break;
					case 0b100: // pixel, take it
					case 0b101: // pixel, bg on, fore off
						vram[(dest->base_address + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = px;
						break;
					case 0b110: // pixel, bg off, fore on
					case 0b111:
						vram[(dest->base_address + ((dest_y + src->y) * dest->w) + dest_x + src->x) & VRAM_SIZE_MASK] = src->color_indices[1];
						break;
				}

				pixel_saldo--;
			}
		}
	}

	return old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::tile_blit(const uint8_t s, const uint8_t d, const uint8_t _ts)
{
	const surface_t *src = &surface[s & 0b1111];
	surface_t *dst = &surface[d & 0b1111];
	const surface_t *ts = &surface[_ts & 0b1111];

	uint32_t pixelcount = 0;

	surface_t source = *src;

	uint8_t dw = (src->flags_1 & FLAGS1_DBLWIDTH) ? 1 : 0;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) ? 1 : 0;

	source.x = ts->x;
	source.y = ts->y;
	uint32_t tile_index = ts->base_address;
	uint32_t fg_color_index = tile_index + (ts->w * ts->h);
	uint32_t bg_color_index = tile_index + (2 * ts->w * ts->h);

	bool use_fixed_bg = ts->flags_0 & 0b01 ? true : false;
	bool use_fixed_fg = ts->flags_0 & 0b10 ? true : false;

	for (int y = 0; y < ts->h; y++) {
		for (int x = 0; x < ts->w; x++) {
			source.index = vram[tile_index++ & VRAM_SIZE_MASK];
			source.color_indices[0] = use_fixed_bg ? ts->color_indices[0] : vram[bg_color_index++ & VRAM_SIZE_MASK];
			source.color_indices[1] = use_fixed_fg ? ts->color_indices[1] : vram[fg_color_index++ & VRAM_SIZE_MASK];
			pixelcount += blit(&source, dst);
			source.x += (source.w << dw);
		}
		source.x = ts->x;
		source.y += (source.h << dh);
	}

	return pixelcount;
}

uint32_t blitter_ic::clear_surface(const uint8_t col, const uint8_t dest)
{
	surface_t *d = &surface[dest & 0xf];
	uint32_t pixels = d->w * d->h;
	uint32_t old_pixel_saldo = pixel_saldo;

	for (uint32_t i=0; i < pixels; i++) {
		if (pixel_saldo) {
			vram[(d->base_address + i) & VRAM_SIZE_MASK] = col;
			pixel_saldo--;
		} else {
			break;
		}
	}

	return old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::pset(int16_t x0, int16_t y0, uint8_t c, uint8_t d)
{
	vram[(surface[d & 0b1111].base_address + (y0 * surface[d & 0b1111].w) + x0) & VRAM_SIZE_MASK] = c;
	pixel_saldo++;
	return 1;
}

uint32_t blitter_ic::line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d)
{
	surface_t *s = &surface[d & 0b1111];

	uint32_t old_pixel_saldo = pixel_saldo;

	int16_t dx = abs(x1 - x0);
	int16_t dy = abs(y1 - y0);
	int16_t sx, sy;

	sx = x0 < x1 ? 1 : -1;
	sy = y0 < y1 ? 1 : -1;

	int16_t err = dx - dy;

	while (x0 != x1 || y0 != y1) {
		if ((x0 >= 0) && (x0 < s->w) && (y0 >= 0) && (y0 < s->h)) {
			if (pixel_saldo) {
				pixel_saldo--;
				vram[(s->base_address + (y0 * s->w) + x0) & VRAM_SIZE_MASK] = c;
			}
		}

		int e2 = 2 * err;
		if (e2 > -dy) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}

	/*
	 * Draw endpoint
	 */
	if ((x0 >= 0) && (x0 < s->w) && (y0 >= 0) && (y0 < s->h)) {
		if (pixel_saldo) {
			   pixel_saldo--;
			   vram[(s->base_address + (y0 * s->w) + x0) & VRAM_SIZE_MASK] = c;
		}
	}

	return  old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d)
{
	uint32_t pixels{0};

	pixels += line(x0, y0, x1, y0, c, d);
	pixels += line(x1, y0, x1, y1, c, d);
	pixels += line(x1, y1, x0, y1, c, d);
	pixels += line(x0, y1, x0, y0, c, d);

	return pixels;
}

uint32_t blitter_ic::solid_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t c, uint8_t d)
{
	uint32_t pixels{0};

	auto swap = [](int16_t &a, int16_t &b) { int16_t c = a; a = b; b = c; };

	if (y0 > y1) swap(y0, y1);

	for (int16_t y=y0; y<=y1; y++) {
		pixels += line(x0, y, x1, y, c, d);
	}

	return pixels;
}

//uint32_t blitter_ic::flood_fill(int16_t x0, int16_t y0, uint8_t c, uint8_t d)
//{
//	uint32_t pixels{0};
//	surface_t *s = &surface[d & 0b1111];
//
//	uint8_t target_color = (s->base_page << 8) | (
//	return pixels;
//}

uint8_t blitter_ic::io_read8(uint16_t address)
{
	switch (address & 0xff) {
		case 0x02: return src_surface;
		case 0x03: return dst_surface;
		case 0x04: return tile_surface;
		case 0x05: return draw_color;

		case 0x08: return (((uint16_t)x0) & 0xff00) >> 8;
		case 0x09: return ((uint16_t)x0) & 0xff;
		case 0x0a: return (((uint16_t)y0) & 0xff00) >> 8;
		case 0x0b: return ((uint16_t)y0) & 0xff;
		case 0x0c: return (((uint16_t)x1) & 0xff00) >> 8;
		case 0x0d: return ((uint16_t)x1) & 0xff;
		case 0x0e: return (((uint16_t)y1) & 0xff00) >> 8;
		case 0x0f: return ((uint16_t)y1) & 0xff;

		default: return 0x00;
	}
}

void blitter_ic::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0xff) {
		case 0x01:
			// control register
			switch (value) {
				case 0b00000001: blit(src_surface, dst_surface); break;
				case 0b00000010: tile_blit(src_surface, dst_surface, tile_surface); break;
				case 0b00000100: clear_surface(draw_color, dst_surface); break;
				case 0b00001000: pset(x0, y0, draw_color, dst_surface); break;
				case 0b00010000: line(x0, y0, x1, y1, draw_color, dst_surface); break;
				case 0b00100000: rectangle(x0, y0, x1, y1, draw_color, dst_surface); break;
				case 0b01000000: solid_rectangle(x0, y0, x1, y1, draw_color, dst_surface); break;
				default: break;
			}
			break;
		case 0x02: src_surface = value & 0b1111; break;
		case 0x03: dst_surface = value & 0b1111; break;
		case 0x04: tile_surface = value & 0b1111; break;
		case 0x05: draw_color = value; break;

		case 0x08: x0 = (int16_t)((((uint16_t)x0) & 0x00ff) | (value << 8)); break;
		case 0x09: x0 = (int16_t)((((uint16_t)x0) & 0xff00) | value);        break;
		case 0x0a: y0 = (int16_t)((((uint16_t)y0) & 0x00ff) | (value << 8)); break;
		case 0x0b: y0 = (int16_t)((((uint16_t)y0) & 0xff00) | value);        break;
		case 0x0c: x1 = (int16_t)((((uint16_t)x1) & 0x00ff) | (value << 8)); break;
		case 0x0d: x1 = (int16_t)((((uint16_t)x1) & 0xff00) | value);        break;
		case 0x0e: y1 = (int16_t)((((uint16_t)y1) & 0x00ff) | (value << 8)); break;
		case 0x0f: y1 = (int16_t)((((uint16_t)y1) & 0xff00) | value);        break;

		default: break;
	}
}

uint8_t blitter_ic::io_surfaces_read8(uint16_t address)
{
	int8_t no = (address & 0xf0) >> 4;

	switch (address & 0xf) {
		case 0x0: return (((uint16_t)surface[no].x) & 0xff00) >> 8;
		case 0x1: return ((uint16_t)surface[no].x) & 0xff;
		case 0x2: return (((uint16_t)surface[no].y) & 0xff00) >> 8;
		case 0x3: return ((uint16_t)surface[no].y) & 0xff;
		case 0x4: return (surface[no].w & 0xff00) >> 8;
		case 0x5: return surface[no].w & 0xff;
		case 0x6: return (surface[no].h & 0xff00) >> 8;
		case 0x7: return surface[no].h & 0xff;
//		case 0x8: return 0x00;
		case 0x9: return (surface[no].base_address & 0x00ff0000) >> 16;
		case 0xa: return (surface[no].base_address & 0x0000ff00) >>  8;
		case 0xb: return surface[no].base_address & 0x000000ff;
		case 0xc: return surface[no].flags_0;
		case 0xd: return surface[no].flags_1;
		case 0xe: return surface[no].flags_2;
		case 0xf: return surface[no].index;
		default:  return 0x00;
	}
}

void blitter_ic::io_surfaces_write8(uint16_t address, uint8_t value)
{
	int8_t no = (address & 0xf0) >> 4;

	switch (address & 0xf) {
		case 0x0: surface[no].x = (int16_t)((((uint16_t)surface[no].x) & 0x00ff) | (value << 8)); break;
		case 0x1: surface[no].x = (int16_t)((((uint16_t)surface[no].x) & 0xff00) | value);        break;
		case 0x2: surface[no].y = (int16_t)((((uint16_t)surface[no].y) & 0x00ff) | (value << 8)); break;
		case 0x3: surface[no].y = (int16_t)((((uint16_t)surface[no].y) & 0xff00) | value);        break;
		case 0x4: surface[no].w = (surface[no].w & 0x00ff) | (value << 8); break;
		case 0x5: surface[no].w = (surface[no].w & 0xff00) | value;        break;
		case 0x6: surface[no].h = (surface[no].h & 0x00ff) | (value << 8); break;
		case 0x7: surface[no].h = (surface[no].h & 0xff00) | value;        break;
//		case 0x8: break;
		case 0x9: surface[no].base_address = (surface[no].base_address & 0x0000ffff) | (value << 16); break;
		case 0xa: surface[no].base_address = (surface[no].base_address & 0x00ff00ff) | (value << 8);  break;
		case 0xb: surface[no].base_address = (surface[no].base_address & 0x00ffff00) | value;         break;
		case 0xc: surface[no].flags_0 = value & 0b00110011; break;
		case 0xd: surface[no].flags_1 = value & 0b01110011; break;
		case 0xe: surface[no].flags_2 = value & 0b00000111; break;
		case 0xf: surface[no].index = value; break;
		default:  break;
	}
}

uint8_t blitter_ic::io_color_indices_read8(uint16_t address)
{
	uint8_t no = (address & 0xf0) >> 4;
	return surface[no].color_indices[address & 0xf];
}

void blitter_ic::io_color_indices_write8(uint16_t address, uint8_t value)
{
	uint8_t no = (address & 0xf0) >> 4;
	surface[no].color_indices[address & 0xf] = value;
}

void blitter_ic::update_framebuffer(uint32_t base_address)
{
	for (int i = 0; i < PIXELS; i++) {
		framebuffer[i] = palette[vram[(base_address + i) & VRAM_SIZE_MASK]];
	}
}
