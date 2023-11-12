#include <cstdio>
#include <cstdint>
#include "common.hpp"
#include "blitter.hpp"
#include "vrEmu6502.h"
#include <SDL2/SDL.h>

uint8_t memory[65536];

uint8_t read8(uint16_t address, bool is_debug)
{
	return memory[address];
}

void write8(uint16_t address, uint8_t val)
{
	memory[address] = val;
}

void create_scanlines_texture(SDL_Texture *slt);

int main()
{
	memory[0xfffc] = 0x00;
	memory[0xfffd] = 0x02;
	
	memory[0x0200] = 0xa9;	// lda #$21
	memory[0x0201] = 0x21;
	memory[0x0202] = 0x85;	// sta $40
	memory[0x0203] = 0x40;
	memory[0x0204] = 0xe6;	// inc $40
	memory[0x0205] = 0x40;
	memory[0x0206] = 0x80;	// bra $0204
	memory[0x0207] = 0xfc;
	
	uint32_t buffer[PIXELS];
	
	/*
	 * RGB332
	 * https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_graphics
	 */
	
	uint8_t basis = 16;
	uint8_t rest = 255 - basis;
	uint32_t palette[256];
	//const uint8_t values[8] = { 0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff };
	for (int i=0; i<256; i++) {
		uint32_t r = (basis + (rest * ((i & 0b11100000) >> 5)) / 7) << 16;
		uint32_t g = (basis + (rest * ((i & 0b00011100) >> 2)) / 7) << 8;
		uint32_t b = (basis + (rest * ((i & 0b00000011) >> 0)) / 3);
		palette[i] = 0xff000000 | r | g | b;
		printf("palette[%02x] = 0x%08x\n", i, palette[i]);
		memory[(MAX_PIXELS_PER_SCANLINE*16) + (((i & 0b11100000)>>5)*MAX_PIXELS_PER_SCANLINE) + (i&0b11111)] = i;
	}
	
	blitter_ic *blitter = new blitter_ic();
	
	VrEmu6502 *cpu;
	cpu = vrEmu6502New(CPU_W65C02, read8, write8);
	
	SDL_Init(SDL_INIT_EVERYTHING);
	
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	uint8_t scaling = 1;
	while (((scaling * MAX_PIXELS_PER_SCANLINE) < dm.w) && ((scaling * MAX_SCANLINES) < dm.h)) scaling++;
	scaling--;
	printf("scaling = %i\n", scaling);
	
	SDL_Window *window = SDL_CreateWindow("punch", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAX_PIXELS_PER_SCANLINE*scaling, MAX_SCANLINES*scaling, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_Texture *screen_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	SDL_Texture *scanlines_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, MAX_PIXELS_PER_SCANLINE, 4*MAX_SCANLINES);
	create_scanlines_texture(scanlines_texture);
	
	//SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	
	bool running = true;
	SDL_Event my_event;
	
	uint32_t frames = 0;
	
	int32_t cycles = 0;
	
	while (running) {
		frames++;
		while (SDL_PollEvent(&my_event) != 0 ) {
			if(my_event.type == SDL_QUIT ) {
				running = false;
			}
		}
		
		cycles += 985248;
		while (cycles > 0) {
			vrEmu6502Tick(cpu);
			cycles--;
		}
		
		for (auto i=0; i < PIXELS; i++) {
			buffer[i] = palette[memory[i]];
		}
		SDL_UpdateTexture(screen_texture, NULL, buffer, sizeof(uint32_t) * MAX_PIXELS_PER_SCANLINE);
		
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
		
		SDL_SetTextureAlphaMod(scanlines_texture, 64);
		SDL_RenderCopy(renderer, scanlines_texture, NULL, NULL);

		SDL_RenderPresent(renderer);

	}
	
	SDL_DestroyTexture(screen_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	
	SDL_Quit();
	vrEmu6502Destroy(cpu);
	
	delete blitter;
	
	printf("%i frames\n", frames);
	
	return 0;
}

void create_scanlines_texture(SDL_Texture *slt)
{
	uint32_t scanline_buffer[PIXELS * 4];
	
	SDL_SetTextureBlendMode(slt, SDL_BLENDMODE_BLEND);
	
	for (int i=0; i < 4*MAX_SCANLINES; i++) {
		for (int j=0; j < MAX_PIXELS_PER_SCANLINE; j++) {
			uint32_t color = 0x00000000;
			if ((i % 4) == 0 || (i % 4) == 3) {
				color = 0xff000000;
			}
			scanline_buffer[(i * MAX_PIXELS_PER_SCANLINE) + j] = color;
		}
	}
	SDL_UpdateTexture(slt, NULL, scanline_buffer, MAX_PIXELS_PER_SCANLINE * sizeof(uint32_t));
}
