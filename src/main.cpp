#include <cstdio>
#include <cstdint>
#include "common.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include <SDL2/SDL.h>

void create_scanlines_texture(SDL_Texture *slt);

void dump(mc6809 *m);

int main()
{
	core_t core;
	core.reset();
	
	debugger_t debugger;
	
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	uint8_t scaling = 1;
	while (((scaling * MAX_PIXELS_PER_SCANLINE) < dm.w) && ((scaling * MAX_SCANLINES) < dm.h)) scaling++;
	scaling = (3 * scaling) / 4;
	printf("scaling = %i\n", scaling);
	
	/*
	 * TODO: Need SDL_WINDOW_ALWAYS_ON_TOP?
	 */
	SDL_Window *window = SDL_CreateWindow("punch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_PIXELS_PER_SCANLINE*scaling, MAX_SCANLINES*scaling, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_Texture *screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_Texture *debugger_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_Texture *scanlines_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, MAX_PIXELS_PER_SCANLINE, 4*MAX_SCANLINES);
	create_scanlines_texture(scanlines_texture);
	SDL_RenderSetLogicalSize(renderer, 320, 180);	// keeps right aspect ratio
	
	/*
	 * ??
	 */
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	
	/*
	 * Make sure mouse cursor isn't visible
	 */
	SDL_ShowCursor(SDL_DISABLE);
	
	bool running = true;
	SDL_Event my_event;
	
	uint32_t frames = 0;
	
	int32_t cycles = 0;
	
	int16_t dx = 1;
	int16_t dy = 1;
	
	SDL_Rect viewer;
	viewer.w = 128;
	viewer.h = 72;
	viewer.x = 192;
	viewer.y = 0;
	
	while (running) {
		frames++;
		while (SDL_PollEvent(&my_event) != 0 ) {
			if(my_event.type == SDL_QUIT ) {
				running = false;
			}
		}
		
		cycles += 985248;
		
		core.run(cycles);
		
		/*
		 * Clear framebuffer
		 */
		core.blitter->clear_surface(&core.blitter->screen);
		
		core.blitter->blit(&core.blitter->turn_text, &core.blitter->screen);
		core.blitter->blit(&core.blitter->bruce, &core.blitter->screen);
		core.blitter->blit(&core.blitter->punch, &core.blitter->screen);
		
		/*
		 * blit
		 */
		core.blitter->blit(&core.blitter->blob, &core.blitter->screen);
		
		core.blitter->blob.x += dx;
		core.blitter->blob.y += dy;
		
		if ((core.blitter->blob.x > 316) || (core.blitter->blob.x < 1)) dx = -dx;
		if ((core.blitter->blob.y > 174) || (core.blitter->blob.y < 1)) dy = -dy;
		
		debugger.redraw();
		
		/*
		 * pointer for texture straight in memory!
		 * TODO: boundary checking
		 */
		SDL_UpdateTexture(screen_texture, NULL, &core.blitter->vram[(core.blitter->framebuffer_bank & 0x0f) << 16], sizeof(uint8_t) * MAX_PIXELS_PER_SCANLINE);
		SDL_UpdateTexture(debugger_texture, NULL, &debugger.blitter->vram[(debugger.blitter->framebuffer_bank & 0x0f) << 16], sizeof(uint8_t) * MAX_PIXELS_PER_SCANLINE);
		
		/*
		 * TODO: Consider background color from blitter here?
		 */
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		
		SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
		SDL_RenderCopy(renderer, debugger_texture, NULL, NULL);
		
		SDL_SetTextureAlphaMod(scanlines_texture, 64);
		SDL_RenderCopy(renderer, scanlines_texture, NULL, NULL);
		
		SDL_RenderCopy(renderer, screen_texture, NULL, &viewer);

		SDL_RenderPresent(renderer);
	}
	
	SDL_DestroyTexture(debugger_texture);
	SDL_DestroyTexture(screen_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	SDL_Quit();
	
	printf("%i frames\n", frames);
	
	return 0;
}

void create_scanlines_texture(SDL_Texture *slt)
{
	uint16_t scanline_buffer[PIXELS * 4];
	
	SDL_SetTextureBlendMode(slt, SDL_BLENDMODE_BLEND);
	
	for (int i=0; i < 4*MAX_SCANLINES; i++) {
		for (int j=0; j < MAX_PIXELS_PER_SCANLINE; j++) {
			uint16_t color = 0x0000;
			if ((i % 4) == 0 || (i % 4) == 3) {
				color = 0xf000;
			}
			scanline_buffer[(i * MAX_PIXELS_PER_SCANLINE) + j] = color;
		}
	}
	SDL_UpdateTexture(slt, NULL, scanline_buffer, MAX_PIXELS_PER_SCANLINE * sizeof(uint16_t));
}

//void dump(mc6809 *m)
//{
//	printf(" pc  dp ac:br  xr   yr   us   sp  efhinzvc\n%04x %02x %02x %02x %04x %04x %04x %04x %c%c%c%c%c%c%c%c\n",
//	       m->get_pc(),
//	       m->get_dp(),
//	       m->get_ac(),
//	       m->get_br(),
//	       m->get_xr(),
//	       m->get_yr(),
//	       m->get_us(),
//	       m->get_sp(),
//	       (m->get_cc() & 0x80) ? '1' : '0',
//	       (m->get_cc() & 0x40) ? '1' : '0',
//	       (m->get_cc() & 0x20) ? '1' : '0',
//	       (m->get_cc() & 0x10) ? '1' : '0',
//	       (m->get_cc() & 0x08) ? '1' : '0',
//	       (m->get_cc() & 0x04) ? '1' : '0',
//	       (m->get_cc() & 0x02) ? '1' : '0',
//	       (m->get_cc() & 0x01) ? '1' : '0');
//	char buffer[64];
//	m->disassemble_instruction(buffer, m->get_pc());
//	printf("%s\n", buffer);
//}
