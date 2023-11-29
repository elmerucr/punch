#include "debugger.hpp"
#include "common.hpp"
#include <cstdio>

debugger_t::debugger_t(core_t *c)
{
	core = c;
	
	blitter = new blitter_ic();
	blitter->framebuffer_bank = 0x0e;
	blitter->framebuffer.bg_col = 0x00;
	
	font_surface.flags_0 = 0b01000011;
	font_surface.flags_1 = 0b00000000;
	font_surface.keycolor = C64_BLUE;
	font_surface.fg_col = 0b10011110;
	font_surface.w = 4;
	font_surface.h = 6;

	character_screen.columns = MAX_PIXELS_PER_SCANLINE / 4;
	character_screen.rows = MAX_SCANLINES / 6;
	character_screen.base = 0x10000;
	character_screen.x = 0;
	character_screen.y = 0;
	
	terminal = new terminal_t(&character_screen, blitter);
	terminal->clear();
	terminal->printf("punch v%i.%i.%i (C)%i elmerucr\n",
	       PUNCH_MAJOR_VERSION,
	       PUNCH_MINOR_VERSION,
	       PUNCH_BUILD, PUNCH_YEAR);
	terminal->puts("\n\n\n\n.,0c000 lda #$0e");
	terminal->puts("\n.,0c002 tfr a,b\n\n\n\n\n\n\n\n");
	
	char output[256];
	core->cpu->status(output, 256);
	terminal->puts(output);
}

debugger_t::~debugger_t()
{
	delete terminal;
	delete blitter;
}

void debugger_t::redraw()
{
	blitter->clear_surface(&blitter->framebuffer);
	blitter->tile_blit(&font_surface, &blitter->framebuffer, &character_screen);
}
