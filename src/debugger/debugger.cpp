#include "debugger.hpp"
#include "common.hpp"
#include <cstdio>

debugger_t::debugger_t()
{
	blitter = new blitter_ic();
	blitter->framebuffer_bank = 0x0e;

	screen.base = blitter->framebuffer_bank << 16;
	screen.bg_col = C64_BLUE;
	screen.w = MAX_PIXELS_PER_SCANLINE;
	screen.h = MAX_SCANLINES;
	
	font.flags_0 = 0b01000011;
	font.flags_1 = 0b00000000;
	font.keycolor = C64_BLUE;
	font.fg_col = C64_LIGHTBLUE;
	font.w = 4;
	font.h = 6;

	terminal.columns = MAX_PIXELS_PER_SCANLINE / 4;
	terminal.rows = MAX_SCANLINES / 6;
	terminal.base = 0x10000;
	terminal.x = 0;
	terminal.y = 0;

	for (int i=0; i < (terminal.columns * terminal.rows); i++) {
		blitter->vram[terminal.base + i] = 0x20;
	}
	snprintf((char *)&blitter->vram[terminal.base], 80, ".,0c000 lda #$0e");
}

debugger_t::~debugger_t()
{
	delete blitter;
}

void debugger_t::redraw()
{
	blitter->clear_surface(&screen);
	blitter->tile_blit(&font, &screen, &terminal);
}
