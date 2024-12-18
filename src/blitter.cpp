// ---------------------------------------------------------------------
// blitter.cpp
// punch
//
// Copyright © 2023-2024 elmerucr. All rights reserved.
// ---------------------------------------------------------------------

#include "blitter.hpp"
#include "common.hpp"
#include <cstdio>
#include <cmath>

blitter_ic::blitter_ic()
{
	vram = new uint8_t[VRAM_SIZE];
}

blitter_ic::~blitter_ic()
{
	delete [] vram;
}

void blitter_ic::reset()
{
	for (int i = 0; i < VRAM_SIZE; i++) {
		vram[i] = (i & 0x40) ? 0xfc : 0x00;
	}

	// A palette using RRGGBBII system. R, G and B use two bits and have
	// 4 levels each (0.00, 0.33, 0.66 and 1.00 of max). On top of that,
	// the intensity level (II) is shared between all channels.
	//
	// Final color levels are RR * II, GG * II and BB * II.
	//
	// II is not linear, see below. This system results in a nice palette
	// with many dark shades as well to choose from (compared to RGB332).
	//
	// Inspired by: https://www.bigmessowires.com/2008/07/04/video-palette-setup/
	for (int i = 0; i < 256; i++) {
		uint32_t r = (i & 0b11000000) >> 6;
		uint32_t g = (i & 0b00110000) >> 4;
		uint32_t b = (i & 0b00001100) >> 2;
		uint32_t s = (i & 0b00000011) >> 0;
		uint32_t factor = 0;

		switch (s) {
			case 0b00: factor =  5; break;
			case 0b01: factor =  8; break;
			case 0b10: factor = 12; break;
			case 0b11: factor = 15; break;
		}

		// TODO: Unfortunately, rounding here is key to the colors that
		// can be seen. Maybe optimize this somehow?
		r = 17 * ((factor * r) / 3);
		g = 17 * ((factor * g) / 3);
		b = 17 * ((factor * b) / 3);

		vram[palette_addr + (i << 2) + 0] = 0xff;
		vram[palette_addr + (i << 2) + 1] = r;
		vram[palette_addr + (i << 2) + 2] = g;
		vram[palette_addr + (i << 2) + 3] = b;
	}

	vram[palette_addr] = 0x00; // first color is transparent (empty) 0x00000000

	for (int i=0; i<256; i++) {
		for (int j=0; j<16; j++) {
			surface[j].color_table[i] = i;
		}
	}

	surface[0].w = MAX_PIXELS_PER_SCANLINE;
	surface[0].h = MAX_SCANLINES;
	surface[0].base_address = FRAMEBUFFER_ADDRESS;
	surface[0].flags_0 = 0x40;
	surface[0].flags_1 = 0x00;
	surface[0].flags_2 = 0x00;
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
	 * Calculate bitshifts for double width and height
	 */
	uint8_t dw = src->flags_1 & FLAGS1_DBLWIDTH;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) >> 2;

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
	uint8_t *memory;		// memory is start of an array to 8 bit color numbers
	uint32_t memory_mask;	// mask used when referring to this memory
	uint32_t start_address;

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
			memory_mask = font_4x6.mask;
			start_address = 0;
			break;
		case 0b100:
			memory = font_cbm_8x8.data;
			memory_mask = font_cbm_8x8.mask;
			start_address = 0;
			break;
	}

	/*
	 * Based on source index (like a sprite pointer), find an offset to
	 * the start_address
	 */
	uint32_t offset = (src->index * src->w * src->h);

	/*
	 * Get color mode of src surface:
	 *
	 * 0b000 =  1 bit
	 * 0b001 =  2 bit
	 * 0b010 =  4 bit
	 * 0b011 =  8 bit
	 * 0b100 = 32 bit
	 */
	uint8_t color_mode = (src->flags_0 & 0b01110000) >> 4;

	for (int y = starty; y < endy; y++) {
		for (int x = startx; x < endx; x++) {
			if (pixel_saldo) {
				/*
				 * Adjust placement locations if needed
				 */
				int16_t dest_x, dest_y;

				if (src->flags_1 & FLAGS1_HOR_FLIP) dest_x = (src->w << dw) - 1 - x; else dest_x = x;
				if (src->flags_1 & FLAGS1_VER_FLIP) dest_y = (src->h << dh) - 1 - y; else dest_y = y;
				if (src->flags_1 & FLAGS1_X_Y_FLIP) swap(dest_x, dest_y);

				/*
				 * Adjust offset to current values of x and y.
				 */
				uint32_t adjusted_offset = offset + (x >> dw) + (src->w * (y >> dh));	// offset can't change during the for loops!

				/*
				 * Index where pixel source information can be found
				 */
				uint8_t color_index{0};

				if (color_mode < 0b100) {
					// Color selection, first step
					color_index = memory[(start_address + (adjusted_offset / indexed_color_modes[color_mode].pixels_per_byte)) & memory_mask];

					// Depending on number of bits per pixel, there will do a bitshift
					color_index >>= indexed_color_modes[color_mode].bits_per_pixel * (indexed_color_modes[color_mode].pixels_per_byte - (adjusted_offset % indexed_color_modes[color_mode].pixels_per_byte) - 1);

					// And use the correct mask
					color_index &= indexed_color_modes[color_mode].mask;

					// Lookup final color in table
					color_index = src->color_table[color_index];
				}

				/*
				 * Find dst address where result must be stored
				 */
				uint32_t dst = (dest->base_address + ((((dest_y + src->y) * dest->w) + dest_x + src->x) << 2)) & VRAM_SIZE_MASK;

				if (color_mode <= 0b11) {
					// 1, 2, 4 and 8 bit color
					blend(palette_addr + (color_index << 2), dst);
				} else {
					// 32 bit color
					blend((start_address + (adjusted_offset << 2)) & VRAM_SIZE_MASK, dst);
				}
				pixel_saldo--;
			}
		}
	}
	return old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::tile_blit(const uint8_t s, const uint8_t d, const uint8_t _ts)
{
	surface_t *src = &surface[s & 0b1111];
	surface_t *dst = &surface[d & 0b1111];
	const surface_t *ts = &surface[_ts & 0b1111];

	uint32_t pixelcount = 0;

	uint8_t dw = src->flags_1 & FLAGS1_DBLWIDTH;
	uint8_t dh = (src->flags_1 & FLAGS1_DBLHEIGHT) >> 2;

	// save for restoration later on
	int16_t old_x = src->x;
	int16_t old_y = src->y;
	uint8_t old_index = src->index;
	uint8_t old_color_table_0 = src->color_table[0];
	uint8_t old_color_table_1 = src->color_table[1];
	//

	src->x = ts->x;
	src->y = ts->y;
	uint32_t tile_index = ts->base_address;
	uint32_t fg_color_index = tile_index + (ts->w * ts->h);
	uint32_t bg_color_index = tile_index + (2 * ts->w * ts->h);

	bool fixed_bg_color = ts->flags_0 & 0b01 ? true : false;
	bool fixed_fg_color = ts->flags_0 & 0b10 ? true : false;

	for (int y = 0; y < ts->h; y++) {
		for (int x = 0; x < ts->w; x++) {
			src->index = vram[tile_index++ & VRAM_SIZE_MASK];
			src->color_table[0] = fixed_bg_color ? ts->color_table[0] : vram[bg_color_index++ & VRAM_SIZE_MASK];
			src->color_table[1] = fixed_fg_color ? ts->color_table[1] : vram[fg_color_index++ & VRAM_SIZE_MASK];
			pixelcount += blit(src, dst);
			src->x += (src->w << dw);
		}
		src->x = ts->x;				// set to start position
		src->y += (src->h << dh);	// go to next row
	}

	// Restore src
	src->x = old_x;
	src->y = old_y;
	src->index = old_index;
	src->color_table[0] = old_color_table_0;
	src->color_table[1] = old_color_table_1;
	//

	return pixelcount;
}

uint32_t blitter_ic::clear_surface(const uint8_t dest)
{
	surface_t *d = &surface[dest & 0xf];
	uint32_t pixels = d->w * d->h;
	uint32_t old_pixel_saldo = pixel_saldo;

	for (uint32_t i=0; i < pixels; i++) {
		if (pixel_saldo) {
			blend(draw_color_addr, (d->base_address + (i << 2)) & VRAM_SIZE_MASK);
			pixel_saldo--;
		} else {
			break;
		}
	}
	return old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::pset(int16_t x0, int16_t y0, uint8_t d)
{
	if (pixel_saldo) {
		blend(draw_color_addr, (surface[d & 0b1111].base_address + (((y0 * surface[d & 0b1111].w) + x0) << 2)) & VRAM_SIZE_MASK);
		pixel_saldo--;
		return 1;
	}
	return 0;
}

uint32_t blitter_ic::line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d)
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
				blend(draw_color_addr, (s->base_address + (((y0 * s->w) + x0) << 2)) & VRAM_SIZE_MASK);
				pixel_saldo--;
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
			blend(draw_color_addr, (s->base_address + (((y0 * s->w) + x0) << 2)) & VRAM_SIZE_MASK);
			pixel_saldo--;
		}
	}

	return  old_pixel_saldo - pixel_saldo;
}

uint32_t blitter_ic::rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d)
{
	uint32_t pixels{0};

	pixels += line(x0, y0, x1, y0, d);
	pixels += line(x1, y0, x1, y1, d);
	pixels += line(x1, y1, x0, y1, d);
	pixels += line(x0, y1, x0, y0, d);

	return pixels;
}

uint32_t blitter_ic::solid_rectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t d)
{
	uint32_t pixels{0};

	auto swap = [](int16_t &a, int16_t &b) { int16_t c = a; a = b; b = c; };

	if (y0 > y1) swap(y0, y1);

	for (int16_t y=y0; y<=y1; y++) {
		pixels += line(x0, y, x1, y, d);
	}

	return pixels;
}

uint8_t blitter_ic::io_read8(uint16_t address)
{
	switch (address & 0x300) {
		case 0x000:
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

				case 0x10: return 0x00;
				case 0x11: return (vram_peek & 0x00ff0000) >> 16;
				case 0x12: return (vram_peek & 0x0000ff00) >>  8;
				case 0x13: return (vram_peek & 0x000000ff) >>  0;

				case 0x18: return alpha;
				case 0x19: return gamma_red;
				case 0x1a: return gamma_green;
				case 0x1b: return gamma_blue;

				default: return 0x00;
			}
		case 0x100:
			return vram[(vram_peek + (address & 0xff)) & VRAM_SIZE_MASK];
		case 0x200:
			return io_surfaces_read8(address & 0xff);
		default:
			return 0x00;
	}
}

void blitter_ic::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x300) {
		case 0x000:
			switch (address & 0xff) {
				case 0x01:
					// control register
					switch (value) {
						case 0b0000001: blit(src_surface, dst_surface); break;
						case 0b0000010: tile_blit(src_surface, dst_surface, tile_surface); break;
						case 0b0000100: clear_surface(dst_surface); break;
						case 0b0001000: pset(x0, y0, dst_surface); break;
						case 0b0010000: line(x0, y0, x1, y1, dst_surface); break;
						case 0b0100000: rectangle(x0, y0, x1, y1, dst_surface); break;
						case 0b1000000: solid_rectangle(x0, y0, x1, y1, dst_surface); break;
						default: break;
					}
					break;
				case 0x02: src_surface = value & 0b1111; break;
				case 0x03: dst_surface = value & 0b1111; break;
				case 0x04: tile_surface = value & 0b1111; break;
				case 0x05:
					draw_color = value;
					vram[draw_color_addr + 0] = vram[palette_addr + (value << 2) + 0];
					vram[draw_color_addr + 1] = vram[palette_addr + (value << 2) + 1];
					vram[draw_color_addr + 2] = vram[palette_addr + (value << 2) + 2];
					vram[draw_color_addr + 3] = vram[palette_addr + (value << 2) + 3];
					break;
				case 0x08: x0 = (int16_t)((((uint16_t)x0) & 0x00ff) | (value << 8)); break;
				case 0x09: x0 = (int16_t)((((uint16_t)x0) & 0xff00) | value);        break;
				case 0x0a: y0 = (int16_t)((((uint16_t)y0) & 0x00ff) | (value << 8)); break;
				case 0x0b: y0 = (int16_t)((((uint16_t)y0) & 0xff00) | value);        break;
				case 0x0c: x1 = (int16_t)((((uint16_t)x1) & 0x00ff) | (value << 8)); break;
				case 0x0d: x1 = (int16_t)((((uint16_t)x1) & 0xff00) | value);        break;
				case 0x0e: y1 = (int16_t)((((uint16_t)y1) & 0x00ff) | (value << 8)); break;
				case 0x0f: y1 = (int16_t)((((uint16_t)y1) & 0xff00) | value);        break;

				case 0x10: break; // do nothing
				case 0x11: vram_peek = (vram_peek & 0x0000ffff) | (value << 16); break;
				case 0x12: vram_peek = (vram_peek & 0x00ff00ff) | (value <<  8); break;
				case 0x13: vram_peek = (vram_peek & 0x00ffff00) | (value <<  0); break;

				case 0x18: alpha = value; break;
				case 0x19: gamma_red = value; break;
				case 0x1a: gamma_green = value; break;
				case 0x1b: gamma_blue = value; break;

				default: break;
			}
			break;
		case 0x100:
			vram[(vram_peek + (address & 0xff)) & VRAM_SIZE_MASK] = value;
			break;
		case 0x200:
			io_surfaces_write8(address & 0xff, value);
			break;
		default:
			break;
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

	if (no) {
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
			case 0xc:
				surface[no].flags_0 = value & 0b01110011;
				// proper check for 32bit = 0b0100--- only!
				if (surface[no].flags_0 & 0b01000000) surface[no].flags_0 &= 0b11001111;
				break;
			case 0xd: surface[no].flags_1 = value & 0b01111111; break;
			case 0xe: surface[no].flags_2 = value & 0b00000111; break;
			case 0xf: surface[no].index = value; break;
			default:  break;
		}
	}
}

uint8_t blitter_ic::io_color_table_read8(uint16_t address)
{
	uint8_t no = (address & 0x0f00) >> 8;
	return surface[no].color_table[address & 0xff];
}

void blitter_ic::io_color_table_write8(uint16_t address, uint8_t value)
{
	uint8_t no = (address & 0x0f00) >> 8;
	surface[no].color_table[address & 0xff] = value;
}
