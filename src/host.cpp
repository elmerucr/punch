/*
 * host.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "host.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include <thread>
#include <chrono>
#include <unistd.h>
#include <iostream>
#include <filesystem>

host_t::host_t(system_t *s)
{
	core_buffer = new uint32_t[((2 * MAX_SCANLINES) + 1) * MAX_PIXELS_PER_SCANLINE];
	for (int i=0; i<2*PIXELS; i++) {
		core_buffer[i] = 0;
	}

	debugger_buffer = new uint32_t[((2 * MAX_SCANLINES) + 1) * MAX_PIXELS_PER_SCANLINE];
	for (int i=0; i<2*PIXELS; i++) {
		debugger_buffer[i] = 0;
	}

	system = s;

	SDL_Init(SDL_INIT_EVERYTHING);

	/*
	 * Each call to SDL_PollEvent invokes SDL_PumpEvents() that
	 * updates this array.
	 */
	sdl_keyboard_state = SDL_GetKeyboardState(NULL);

	for (int i=0; i<128; i++) keyboard_state[i] = 0;

	SDL_version compiled;
	SDL_VERSION(&compiled);
	printf("[SDL] Compiled against SDL version %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);

	SDL_version linked;
	SDL_GetVersion(&linked);
	printf("[SDL] Linked against SDL version %d.%d.%d\n", linked.major, linked.minor, linked.patch);

	//std::filesystem::path p = SDL_GetBasePath();
	//std::cout << p << std::endl;

	char *base_path = SDL_GetBasePath();
	printf("[SDL] Base path is: %s\n", base_path);
	SDL_free(base_path);

	sdl_preference_path = SDL_GetPrefPath("elmerucr", "punch");
	printf("[SDL] Preference path is: %s\n", sdl_preference_path);

#if defined(__APPLE__)
	home = getenv("HOME");
	printf("[host] User homedir is: %s\n", home);
#elif defined(__linux)
	home = getenv("HOME");
	printf("[host] User homedir is: %s\n", home);
#else
#   error "Unknown compiler"
#endif

	audio_init();
	video_init();
}

host_t::~host_t()
{
	delete [] debugger_buffer;
	delete [] core_buffer;

	video_stop();
	audio_stop();
	SDL_Quit();
}

void host_t::audio_init()
{
	/*
	 * Print the list of audio backends
	 */
	int numAudioDrivers = SDL_GetNumAudioDrivers();
	printf("[SDL] audio backend(s): %d compiled into SDL: ", numAudioDrivers);
	for (int i=0; i<numAudioDrivers; i++) {
		printf(" \'%s\' ", SDL_GetAudioDriver(i));
	}
	printf("\n");

	// What's this all about???
	SDL_zero(audio_spec_want);

	/*
	 * Define audio specification
	 */
	audio_spec_want.freq = SAMPLE_RATE;
	audio_spec_want.format = AUDIO_F32SYS;
	audio_spec_want.channels = 2;
	audio_spec_want.samples = 512;
	audio_spec_want.callback = nullptr;

	/*
	 * Open audio device, allowing any changes to the specification
	 */
	audio_device = SDL_OpenAudioDevice(NULL, 0, &audio_spec_want, &audio_spec_have,
						 SDL_AUDIO_ALLOW_ANY_CHANGE);
	if(!audio_device) {
		printf("[SDL] failed to open audio device: %s\n", SDL_GetError());
		// this is not enough and even wrong...
		// consider a system without audio support?
		SDL_Quit();
	}

	printf("[SDL] audio now using backend '%s'\n", SDL_GetCurrentAudioDriver());
	printf("[SDL] audio information:        want\thave\n");
	printf("[SDL]         frequency         %d\t%d\n", audio_spec_want.freq, audio_spec_have.freq);
	printf("[SDL]         format\n"
	       "[SDL]          float            %s\t%s\n",
	       SDL_AUDIO_ISFLOAT(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISFLOAT(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          signed           %s\t%s\n",
	       SDL_AUDIO_ISSIGNED(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISSIGNED(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          big endian       %s\t%s\n",
	       SDL_AUDIO_ISBIGENDIAN(audio_spec_want.format) ? "yes" : "no",
	       SDL_AUDIO_ISBIGENDIAN(audio_spec_have.format) ? "yes" : "no");
	printf("[SDL]          bitsize          %d\t%d\n",
	       SDL_AUDIO_BITSIZE(audio_spec_want.format),
	       SDL_AUDIO_BITSIZE(audio_spec_have.format));
	printf("[SDL]          channels         %d\t%d\n", audio_spec_want.channels, audio_spec_have.channels);
	printf("[SDL]          samples          %d\t%d\n", audio_spec_want.samples, audio_spec_have.samples);

	audio_bytes_per_sample = SDL_AUDIO_BITSIZE(audio_spec_have.format) / 8;
	printf("[SDL] audio is using %d bytes per sample per channel\n", audio_bytes_per_sample);

	audio_bytes_per_ms = (double)SAMPLE_RATE * audio_spec_have.channels * audio_bytes_per_sample / 1000;
	printf("[SDL] audio is using %f bytes per ms\n", audio_bytes_per_ms);

	audio_running = false;

	audio_start();
}

void host_t::audio_start()
{
	if (!audio_running) {
		printf("[SDL] start audio\n");
		// Unpause audiodevice, and process audiostream
		SDL_PauseAudioDevice(audio_device, 0);
		audio_running = true;
	}
}

void host_t::audio_stop()
{
	if (audio_running) {
		printf("[SDL] stop audio\n");
		// Pause audiodevice
		SDL_PauseAudioDevice(audio_device, 1);
		audio_running = false;
	}
}

void host_t::video_init()
{
	/*
	 * Print the list of video backends
	 */
	int num_video_drivers = SDL_GetNumVideoDrivers();
	printf("[SDL] Display %d video backend(s) compiled into SDL: ", num_video_drivers);
	for (int i=0; i<num_video_drivers; i++) {
		printf(" \'%s\' ", SDL_GetVideoDriver(i));
	}
	printf("\n[SDL] Display now using backend '%s'\n", SDL_GetCurrentVideoDriver());

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);	// some implementations take SDL_GetWindowDisplayIndex(video_window) in stead of 0
	printf("[SDL] Display current desktop dimension: %i x %i\n", dm.w, dm.h);

	int width_scaling = dm.w/MAX_PIXELS_PER_SCANLINE;
	int height_scaling = dm.h/MAX_SCANLINES;
	video_scaling_fullscreen = width_scaling < height_scaling ? width_scaling : height_scaling; // take minimum
	if (video_scaling_fullscreen & 0b1) video_scaling_fullscreen--;	// force even
	printf("[SDL] Video scaling fullscreen will be %i times\n", video_scaling_fullscreen);
	fullscreen_rect.w = video_scaling_fullscreen * MAX_PIXELS_PER_SCANLINE;
	fullscreen_rect.h = video_scaling_fullscreen * MAX_SCANLINES;
	fullscreen_rect.x = (dm.w - fullscreen_rect.w) >> 1;
	fullscreen_rect.y = (dm.h - fullscreen_rect.h) >> 1;
	printf("w:%i h:%i x:%i y:%i\n", fullscreen_rect.w, fullscreen_rect.h, fullscreen_rect.x, fullscreen_rect.y);


	if ((fullscreen_rect.x == 0) || (fullscreen_rect.y == 0)) {
		video_scaling_windowed = video_scaling_fullscreen - 2;
	} else {
		video_scaling_windowed = video_scaling_fullscreen;
	}
	if (video_scaling_windowed < 1) video_scaling_windowed = 1;

	/*
	 * Create window
	 */
	video_window = SDL_CreateWindow("punch", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  video_scaling_windowed * MAX_PIXELS_PER_SCANLINE,
				  video_scaling_windowed * MAX_SCANLINES,
				  SDL_WINDOW_SHOWN |
				  SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_GetWindowSize(video_window, &window_width, &window_height);
	printf("[SDL] Display window dimension: %u x %u pixels\n", window_width, window_height);

	/*
	 * Create renderer and link it to window
	 */
	printf("[SDL] Display refresh rate of current display is %iHz\n", dm.refresh_rate);
	if (dm.refresh_rate == FPS) {
		printf("[SDL] Display: this is equal to the FPS of punch, trying for vsync\n");
		SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	} else {
		printf("[SDL] Display: this differs from the FPS of punch, going for software FPS\n");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}

	/*
	 * Used for clearing the renderer. Black.
	 */
	SDL_SetRenderDrawColor(video_renderer, 0, 0, 0, 255);

	SDL_RendererInfo current_renderer;
	SDL_GetRendererInfo(video_renderer, &current_renderer);
	vsync = (current_renderer.flags & SDL_RENDERER_PRESENTVSYNC) ? true : false;

	printf("[SDL] Renderer Name: %s\n", current_renderer.name);
	printf("[SDL] Renderer %saccelerated\n",
	       (current_renderer.flags & SDL_RENDERER_ACCELERATED) ? "" : "not ");
	printf("[SDL] Renderer vsync is %s\n", vsync ? "enabled" : "disabled");
	printf("[SDL] Renderer does%s support rendering to target texture\n", current_renderer.flags & SDL_RENDERER_TARGETTEXTURE ? "" : "n't");

	/*
	 * Create two textures that are able to refresh very frequently
	 */
	create_core_texture();
	create_debugger_texture();

	// started in windowed mode!
	SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);

	/*
	 * Make sure mouse cursor isn't visible
	 */
	SDL_ShowCursor(SDL_DISABLE);
}

void host_t::video_stop()
{
	SDL_DestroyTexture(debugger_texture);
	SDL_DestroyTexture(core_texture);
	SDL_DestroyRenderer(video_renderer);
	SDL_DestroyWindow(video_window);
}

void host_t::update_core_texture(uint32_t *core)
{
	uint8_t *core_8 = (uint8_t *)core;
	uint8_t *core_8_next = core_8 + (MAX_PIXELS_PER_SCANLINE << 2);
	uint8_t *core_buffer_8 = (uint8_t *)core_buffer;

	if (scanlines) {
		for (int y=0; y<(MAX_SCANLINES - 1); y++) {
			for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {

				core_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 0] = *core_8;
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 1] = *(core_8 + 1);
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 2] = *(core_8 + 2);
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 3] = *(core_8 + 3);
				// odd line
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 0] = scanlines_value;
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 1] = ((*(core_8 + 1)) >> 1) + ((*(core_8_next + 1)) >> 1);
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 2] = ((*(core_8 + 2)) >> 1) + ((*(core_8_next + 2)) >> 1);
				core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 3] = ((*(core_8 + 3)) >> 1) + ((*(core_8_next + 3)) >> 1);

				core_8 += 4;
				core_8_next += 4;
			}
		}

		for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 0] = *core_8;
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 1] = *(core_8 + 1);
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 2] = *(core_8 + 2);
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 3] = *(core_8 + 3);

			core_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 0] = scanlines_value;
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 1] = (*(core_8 + 1)) >> 1;
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 2] = (*(core_8 + 2)) >> 1;
			core_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 3] = (*(core_8 + 3)) >> 1;

			core_8 += 4;
			core_8_next += 4;
		}
	} else {
	// simple "fake" scanlines uses alpha value for each second line
	// // This way, the viewer in debug has transparency values...
	// // Unless, of course, SDL_BLENDMODE_NONE will be used :-)
		for (int y=0; y<MAX_SCANLINES; y++) {
			for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {
				core_buffer[(MAX_PIXELS_PER_SCANLINE * (y << 1)) + x] = *core;
				core_buffer[(MAX_PIXELS_PER_SCANLINE * ((y << 1) + 1)) + x] = *core;
				//*(uint8_t *)&core_buffer[(MAX_PIXELS_PER_SCANLINE * ((y << 1) + 1)) + x] = 160;	// 160 is an alpha value
				core++;
			}
		}
	}

	SDL_UpdateTexture(core_texture, NULL, core_buffer, MAX_PIXELS_PER_SCANLINE * sizeof(uint32_t));
}

void host_t::update_debugger_texture(uint32_t *debugger)
{
	uint8_t *debugger_8 = (uint8_t *)debugger;
	uint8_t *debugger_8_next = debugger_8 + (MAX_PIXELS_PER_SCANLINE << 2);
	uint8_t *debugger_buffer_8 = (uint8_t *)debugger_buffer;

	if (scanlines) {
		for (int y=0; y<(MAX_SCANLINES - 1); y++) {
			for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {
				// core line
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 0] = *debugger_8;
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 1] = *(debugger_8 + 1);
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 2] = *(debugger_8 + 2);
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (y << 3) + (x << 2) + 3] = *(debugger_8 + 3);
				// odd line
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 0] = scanlines_value;
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 1] = ((*(debugger_8 + 1)) >> 1) + ((*(debugger_8_next + 1)) >> 1);
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 2] = ((*(debugger_8 + 2)) >> 1) + ((*(debugger_8_next + 2)) >> 1);
				debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((y << 3) + (1 << 2)) + (x << 2) + 3] = ((*(debugger_8 + 3)) >> 1) + ((*(debugger_8_next + 3)) >> 1);

				debugger_8 += 4;
				debugger_8_next += 4;
			}
		}

		for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 0] = *debugger_8;
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 1] = *(debugger_8 + 1);
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 2] = *(debugger_8 + 2);
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * ((MAX_SCANLINES - 1) << 3) + (x << 2) + 3] = *(debugger_8 + 3);

			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 0] = scanlines_value;
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 1] = (*(debugger_8 + 2)) >> 1;
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 2] = (*(debugger_8 + 2)) >> 1;
			debugger_buffer_8[MAX_PIXELS_PER_SCANLINE * (((MAX_SCANLINES - 1) << 3)  + (1 << 2)) + (x << 2) + 3] = (*(debugger_8 + 3)) >> 1;

			debugger_8 += 4;
			debugger_8_next += 4;
		}
	} else {
	// simple "fake" scanlines uses alpha value for each second line
	// // This way, the viewer in debug has transparency values...
	// // Unless, of course, SDL_BLENDMODE_NONE will be used :-)
		for (int y=0; y<MAX_SCANLINES; y++) {
			for (int x=0; x<MAX_PIXELS_PER_SCANLINE; x++) {
				debugger_buffer[(MAX_PIXELS_PER_SCANLINE * (y << 1)) + x] = *debugger;
				debugger_buffer[(MAX_PIXELS_PER_SCANLINE * ((y << 1) + 1)) + x] = *debugger;
				//*(uint8_t *)&debugger_buffer[(MAX_PIXELS_PER_SCANLINE * ((y << 1) + 1)) + x] = 160;	// 160 is an alpha value
				debugger++;
			}
		}
	}

	SDL_UpdateTexture(debugger_texture, NULL, debugger_buffer, MAX_PIXELS_PER_SCANLINE * sizeof(uint32_t));
}

void host_t::update_screen()
{
	SDL_RenderClear(video_renderer);

	switch (system->current_mode) {
		case DEBUG_MODE:
			SDL_RenderCopy(video_renderer, debugger_texture, NULL, video_fullscreen ? &fullscreen_rect : NULL);
			break;
		case RUN_MODE:
			SDL_RenderCopy(video_renderer, core_texture, NULL, video_fullscreen ? &fullscreen_rect : NULL);
			break;
	}

	if ((system->current_mode == DEBUG_MODE) && viewer_visible) {
		SDL_Rect viewer = {
			(190 * video_scaling_fullscreen) + (video_fullscreen ? fullscreen_rect.x : 0),
			(14 * video_scaling_fullscreen) + (video_fullscreen ? fullscreen_rect.y : 0),
			(video_fullscreen ? video_scaling_fullscreen : video_scaling_windowed) * MAX_PIXELS_PER_SCANLINE / 4,
			(video_fullscreen ? video_scaling_fullscreen : video_scaling_windowed) * MAX_SCANLINES / 4
		};
		SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(video_renderer, core_texture, NULL, &viewer);
		SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_BLEND);
	}

	SDL_RenderPresent(video_renderer);
}

/*
 * System uses the RGB332 palette
 * https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_graphics
 */

void host_t::create_core_texture()
{
	if (core_texture) SDL_DestroyTexture(core_texture);

	core_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB32,
				    SDL_TEXTUREACCESS_STREAMING,
				    MAX_PIXELS_PER_SCANLINE, 2 * MAX_SCANLINES);

	SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_BLEND);
}

void host_t::create_debugger_texture()
{
	if (debugger_texture) SDL_DestroyTexture(debugger_texture);

	debugger_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB32,
				    SDL_TEXTUREACCESS_STREAMING,
				    MAX_PIXELS_PER_SCANLINE, 2 * MAX_SCANLINES);

	SDL_SetTextureBlendMode(debugger_texture, SDL_BLENDMODE_BLEND);
}

enum events_output_state host_t::events_process_events()
{
	enum events_output_state return_value = NO_EVENT;

	SDL_Event event;

	//bool shift_pressed = sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] | sdl2_keyboard_state[SDL_SCANCODE_RSHIFT];
	bool alt_pressed = sdl_keyboard_state[SDL_SCANCODE_LALT] | sdl_keyboard_state[SDL_SCANCODE_RALT];
	//bool gui_pressed   = sdl2_keyboard_state[SDL_SCANCODE_LGUI] | sdl2_keyboard_state[SDL_SCANCODE_RGUI];

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				return_value = KEYPRESS_EVENT;
				if ((event.key.keysym.sym == SDLK_f) && alt_pressed) {
					events_wait_until_key_released(SDLK_f);
					video_toggle_fullscreen();
				} else if ((event.key.keysym.sym == SDLK_s) && alt_pressed ) {
					video_toggle_scanlines();
				} else if ((event.key.keysym.sym == SDLK_r) && alt_pressed) {
					events_wait_until_key_released(SDLK_r);
					system->core->reset();
				} else if( (event.key.keysym.sym == SDLK_q) && alt_pressed ) {
					events_wait_until_key_released(SDLK_q);
					return_value = QUIT_EVENT;
				} else if(event.key.keysym.sym == SDLK_F9) {
					events_wait_until_key_released(SDLK_F9);
					system->switch_mode();
//				} else if(event.key.keysym.sym == SDLK_F10) {
//					hud->toggle_stats();
//				} else if((event.key.keysym.sym == SDLK_w) && alt_pressed) {
//					// TODO: does this always work?
//					if (settings->audio_recording) {
//						hud->show_notification("\nStop recording sound");
//					} else {
//						hud->show_notification("\nStart recording sound");
//					}
//					settings->audio_toggle_recording();
				}
				break;
			case SDL_DROPFILE:
			{
				char *path = event.drop.file;

				std::filesystem::path p(path);
				p.remove_filename();

				std::filesystem::current_path(p);
				printf("\np: %s", p.c_str());
				printf("\ncurrent_path: %s", std::filesystem::current_path().c_str());

				system->debugger->terminal->deactivate_cursor();

				if (chdir(path)) {
					const char *extension = strrchr(path, '.'); extension++;

					if (strcmp(extension, "nut") == 0) {
						system->core->load_squirrel(path);
					} else {
						system->debugger->terminal->printf("\nloading %s", path);
						system->debugger->terminal->printf("\nextension is %s", extension);
						system->core->load_bin();
					}
				} else {
					system->debugger->terminal->printf("\nwarning: %s is a directory", path);
				}

				if (system->current_mode == DEBUG_MODE) system->debugger->prompt();

				system->debugger->terminal->activate_cursor();

				SDL_free(path);
			}
				break;
			case SDL_QUIT:
				return_value = QUIT_EVENT;
				break;
		}
	}

	if (!alt_pressed) {
		(keyboard_state[SCANCODE_ESCAPE      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_ESCAPE      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F1          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F1          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F2          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F2          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F3          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F3          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F4          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F4          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F5          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F5          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F6          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F6          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F7          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F7          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F8          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F8          ] ? 0b1 : 0b0;
		// TODO: what was different here in old E64?
		(keyboard_state[SCANCODE_GRAVE       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_GRAVE       ] ? 0b1 : 0b0;
		//
		(keyboard_state[SCANCODE_1           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_1           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_2           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_2           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_3           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_3           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_4           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_4           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_5           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_5           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_6           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_6           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_7           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_7           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_8           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_8           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_9           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_9           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_0           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_0           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_MINUS       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_MINUS       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_EQUALS      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_EQUALS      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_BACKSPACE   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_BACKSPACE   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_TAB         ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_TAB         ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Q           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Q           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_W           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_W           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_E           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_E           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_R           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_R           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_T           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_T           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Y           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Y           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_U           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_U           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_I           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_I           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_O           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_O           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_P           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_P           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LEFTBRACKET ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LEFTBRACKET ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RIGHTBRACKET] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RIGHTBRACKET] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RETURN      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RETURN      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_A           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_A           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_S           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_S           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_D           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_D           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_F           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_F           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_G           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_G           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_H           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_H           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_J           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_J           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_K           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_K           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_L           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_L           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SEMICOLON   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SEMICOLON   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_APOSTROPHE  ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_APOSTROPHE  ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_BACKSLASH   ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_BACKSLASH   ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LSHIFT      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LSHIFT      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_Z           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_Z           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_X           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_X           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_C           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_C           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_V           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_V           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_B           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_B           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_N           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_N           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_M           ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_M           ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_COMMA       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_COMMA       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_PERIOD      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_PERIOD      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SLASH       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SLASH       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RSHIFT      ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RSHIFT      ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LCTRL       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LCTRL       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_SPACE       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_SPACE       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RCTRL       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RCTRL       ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_LEFT        ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_LEFT        ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_UP          ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_UP          ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_DOWN        ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_DOWN        ] ? 0b1 : 0b0;
		(keyboard_state[SCANCODE_RIGHT       ] <<= 1) |= sdl_keyboard_state[SDL_SCANCODE_RIGHT       ] ? 0b1 : 0b0;
	};

	if (return_value == QUIT_EVENT) printf("[SDL] detected quit event\n");
	return return_value;
}

void host_t::events_wait_until_key_released(SDL_KeyCode key)
{
	SDL_Event event;
	bool wait = true;
	while (wait) {
	    SDL_PollEvent(&event);
	    if ((event.type == SDL_KEYUP) && (event.key.keysym.sym == key)) wait = false;
	    std::this_thread::sleep_for(std::chrono::microseconds(40000));
	}
}

bool host_t::events_yes_no()
{
	SDL_Event event;
	bool checking = true;
	bool return_value = true;
	while (checking) {
		SDL_PollEvent(&event);
		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == SDLK_y) {
				checking = false;
			} else if (event.key.keysym.sym == SDLK_n) {
				return_value = false;
				checking = false;
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	return return_value;
}

void host_t::video_toggle_fullscreen()
{
	video_fullscreen = !video_fullscreen;
	if (video_fullscreen) {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		SDL_GetWindowSize(video_window, &window_width, &window_height);
		SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		printf("[SDL] Fullscreen size: %i x %i\n", window_width, window_height);
	} else {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_RESIZABLE);
		SDL_GetWindowSize(video_window, &window_width, &window_height);
		SDL_RenderSetLogicalSize(video_renderer, window_width, window_height);
		printf("[SDL] Window size: %i x %i\n", window_width, window_height);
	}
}

void host_t::video_toggle_scanlines()
{
	scanlines = !scanlines;
}
