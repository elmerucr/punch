#include "debugger.hpp"
#include "common.hpp"
#include <cstdio>

debugger_t::debugger_t()
{
	blitter = new blitter_ic();
	blitter->framebuffer_bank = 0x0e;
	blitter->framebuffer.bg_col = 0x00;
	
	font.flags_0 = 0b01000011;
	font.flags_1 = 0b00000000;
	font.keycolor = C64_BLUE;
	font.fg_col = 0b10011110;
	font.w = 4;
	font.h = 6;

	character_screen.columns = MAX_PIXELS_PER_SCANLINE / 4;
	character_screen.rows = MAX_SCANLINES / 6;
	character_screen.base = 0x10000;
	character_screen.x = 0;
	character_screen.y = 0;

	for (int i=0; i < (character_screen.columns * character_screen.rows); i++) {
		blitter->vram[character_screen.base + i] = 0x20;
		blitter->vram[character_screen.base + (character_screen.columns * character_screen.rows) + i] = 0b00111000;
	}
	snprintf((char *)&blitter->vram[character_screen.base], 80, ".,0c000 lda #$0e");
}

debugger_t::~debugger_t()
{
	delete blitter;
}

void debugger_t::redraw()
{
	blitter->clear_surface(&blitter->framebuffer);
	blitter->tile_blit(&font, &blitter->framebuffer, &character_screen);
}
