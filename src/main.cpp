#include <cstdio>
#include <cstdint>
#include "common.hpp"
#include "blitter.hpp"
#include "mc6809.hpp"
#include <SDL2/SDL.h>

blitter_ic blitter;

uint8_t read8(uint16_t address)
{
	return blitter.vram[address];
}

void write8(uint16_t address, uint8_t val)
{
	blitter.vram[address] = val;
}

void create_scanlines_texture(SDL_Texture *slt);

void dump(mc6809 *m);

int main()
{
	blitter.vram[0xfffe] = 0x02;
	blitter.vram[0xffff] = 0x00;
	
	blitter.vram[0x0200] = 0x86;	// lda #$21
	blitter.vram[0x0201] = 0x21;
	blitter.vram[0x0202] = 0xb7;	// sta $40
	blitter.vram[0x0203] = 0x00;
	blitter.vram[0x0204] = 0x40;
	blitter.vram[0x0205] = 0x7c;	// inc $40
	blitter.vram[0x0206] = 0x00;
	blitter.vram[0x0207] = 0x40;
	blitter.vram[0x0208] = 0x7e;	// jmp $0205
	blitter.vram[0x0209] = 0x02;
	blitter.vram[0x020a] = 0x05;
	
	mc6809 *cpu;
	cpu = new mc6809(read8, write8);
	dump(cpu);
	cpu->reset();
	dump(cpu);
	
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	uint8_t scaling = 1;
	while (((scaling * MAX_PIXELS_PER_SCANLINE) < dm.w) && ((scaling * MAX_SCANLINES) < dm.h)) scaling++;
	scaling--;
	printf("scaling = %i\n", scaling);
	
	SDL_Window *window = SDL_CreateWindow("punch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_PIXELS_PER_SCANLINE*scaling, MAX_SCANLINES*scaling, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_Texture *screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_Texture *scanlines_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, MAX_PIXELS_PER_SCANLINE, 4*MAX_SCANLINES);
	create_scanlines_texture(scanlines_texture);
	
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	
	bool running = true;
	SDL_Event my_event;
	
	uint32_t frames = 0;
	
	int32_t cycles = 0;
	
	int16_t dx = 1;
	int16_t dy = 1;
	
	while (running) {
		frames++;
		while (SDL_PollEvent(&my_event) != 0 ) {
			if(my_event.type == SDL_QUIT ) {
				running = false;
			}
		}
		
		cycles += 985248;
		while (cycles > 0) {
			cycles -= cpu->execute();
		}
		
		/*
		 * clear framebuffer
		 */
		for (int i=0; i<PIXELS; i++) blitter.vram[i+0x10000] = 0x00;
		
		/*
		 * blit
		 */
		blitter.blit(&blitter.blob, &blitter.screen);

		
		blitter.blob.x += dx;
		blitter.blob.y += dy;
		
		if ((blitter.blob.x > 236) || (blitter.blob.x < 1)) dx = -dx;
		if ((blitter.blob.y > 130) || (blitter.blob.y < 60)) dy = -dy;
		
		/*
		 * pointer for texture straight in memory!
		 * TODO: boundary checking
		 */
		SDL_UpdateTexture(screen_texture, NULL, &blitter.vram[0x0000], sizeof(uint8_t) * MAX_PIXELS_PER_SCANLINE);
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
		
		SDL_SetTextureAlphaMod(scanlines_texture, 32);
		SDL_RenderCopy(renderer, scanlines_texture, NULL, NULL);

		SDL_RenderPresent(renderer);

	}
	
	SDL_DestroyTexture(screen_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	delete cpu;
	
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

void dump(mc6809 *m)
{
	printf(" pc  dp ac:br  xr   yr   us   sp  efhinzvc\n%04x %02x %02x %02x %04x %04x %04x %04x %c%c%c%c%c%c%c%c\n",
	       m->get_pc(),
	       m->get_dp(),
	       m->get_ac(),
	       m->get_br(),
	       m->get_xr(),
	       m->get_yr(),
	       m->get_us(),
	       m->get_sp(),
	       (m->get_cc() & 0x80) ? '1' : '0',
	       (m->get_cc() & 0x40) ? '1' : '0',
	       (m->get_cc() & 0x20) ? '1' : '0',
	       (m->get_cc() & 0x10) ? '1' : '0',
	       (m->get_cc() & 0x08) ? '1' : '0',
	       (m->get_cc() & 0x04) ? '1' : '0',
	       (m->get_cc() & 0x02) ? '1' : '0',
	       (m->get_cc() & 0x01) ? '1' : '0');
	char buffer[64];
	m->disassemble_instruction(buffer, m->get_pc());
	printf("%s\n", buffer);
}
