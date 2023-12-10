
#include "host.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include <thread>
#include <chrono>

host_t::host_t(app_t *a)
{
	app = a;
	
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

	char *base_path = SDL_GetBasePath();
	printf("[SDL] Base path is: %s\n", base_path);
	SDL_free(base_path);

	sdl_preference_path = SDL_GetPrefPath("elmerucr", "punch");
	printf("[SDL] Preference path is: %s\n", sdl_preference_path);
	
	audio_init();
	video_init();
}

host_t::~host_t()
{
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
	printf("[SDL] Display %d video backend(s) compiled into SDL: ",
	       num_video_drivers);
	for (int i=0; i<num_video_drivers; i++)
		printf(" \'%s\' ", SDL_GetVideoDriver(i));
	printf("\n[SDL] Display now using backend '%s'\n", SDL_GetCurrentVideoDriver());

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);	// some implementations take SDL_GetWindowDisplayIndex(video_window) in stead of 0
	printf("[SDL] Display current desktop dimension: %i x %i\n", dm.w, dm.h);
	video_scaling_max = 1;
	while (((video_scaling_max * MAX_PIXELS_PER_SCANLINE) < dm.w) && ((video_scaling_max * ((10 * MAX_SCANLINES) / 9)) < dm.h)) video_scaling_max++;
	video_scaling_max--;
	video_scaling = video_scaling_max;
	//video_scaling = (3 * video_scaling_max) / 4;
	printf("[SDL] Max window scaling is %i, defaulting to %i\n", video_scaling_max, video_scaling);
	
	/*
	 * Create window
	 */
	video_window = SDL_CreateWindow("punch", SDL_WINDOWPOS_CENTERED,
				  SDL_WINDOWPOS_CENTERED,
				  video_scaling * MAX_PIXELS_PER_SCANLINE,
				  video_scaling * ((10 * MAX_SCANLINES) / 9),
				  SDL_WINDOW_SHOWN |
				  SDL_WINDOW_ALLOW_HIGHDPI);

	SDL_GetWindowSize(video_window, &window_width, &window_height);
	printf("[SDL] Display window dimension: %u x %u pixels\n", window_width, window_height);
	
	/*
	 * Create renderer and link it to window
	 */
	printf("[SDL] Display refresh rate of current display is %iHz\n", dm.refresh_rate);
	if (dm.refresh_rate == FPS) {
		printf("[SDL] Display: this is equal to the FPS of E64, trying for vsync\n");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	} else {
		printf("[SDL] Display: this differs from the FPS of E64, going for software FPS\n");
		video_renderer = SDL_CreateRenderer(video_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	}

	SDL_SetRenderDrawColor(video_renderer, 11, 11, 11, 255);
	
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
	core_texture = nullptr;
	debugger_texture = nullptr;
	
	create_core_texture(video_linear_filtering);
	create_debugger_texture(video_linear_filtering);

	/*
	 * Scanlines: A static texture that mimics scanlines
	 */
	scanlines_texture = nullptr;
	create_scanlines_texture(true);
	
	SDL_RenderSetLogicalSize(video_renderer, MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);	// keeps right aspect ratio

	/*
	 * Make sure mouse cursor isn't visible
	 */
	SDL_ShowCursor(SDL_DISABLE);
}

void host_t::video_stop()
{
	SDL_DestroyTexture(scanlines_texture);
	SDL_DestroyTexture(debugger_texture);
	SDL_DestroyTexture(core_texture);
	SDL_DestroyRenderer(video_renderer);
	SDL_DestroyWindow(video_window);
}

//void host_t::update_title()
//{
////	if (machine.mode == E64::PAUSED) {
////		SDL_SetWindowTitle(window, "E64 Debug Mode");
////		// TODO: ?
////		//SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon);
////	} else {
//		SDL_SetWindowTitle(video_window, "E64");
////		// TODO: ?
////		//SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon);
////	}
//}

void host_t::update_core_texture(uint8_t *core)
{
	SDL_UpdateTexture(core_texture, NULL, core, MAX_PIXELS_PER_SCANLINE * sizeof(uint8_t));
}

void host_t::update_debugger_texture(uint8_t *debugger)
{
	SDL_UpdateTexture(debugger_texture, NULL, debugger, MAX_PIXELS_PER_SCANLINE * sizeof(uint8_t));
}

void host_t::update_screen()
{
	SDL_RenderClear(video_renderer);
	
	switch (app->current_mode) {
		case DEBUG_MODE:
			SDL_RenderCopy(video_renderer, debugger_texture, NULL, NULL);
			break;
		case RUN_MODE:
			SDL_RenderCopy(video_renderer, core_texture, NULL, NULL);
			break;
	}
	
	SDL_SetTextureAlphaMod(scanlines_texture, video_scanlines_alpha);
	SDL_RenderCopy(video_renderer, scanlines_texture, NULL, NULL);
	
	if (app->current_mode == DEBUG_MODE) {
		const SDL_Rect viewer = { 256, 0, 64, 36 };
		SDL_RenderCopy(video_renderer, core_texture, NULL, &viewer);
	}

	SDL_RenderPresent(video_renderer);
}

/*
 * System uses the RGB332 palette
 * https://en.wikipedia.org/wiki/List_of_8-bit_computer_hardware_graphics
 */

void host_t::create_core_texture(bool linear_filtering)
{
	if (core_texture) SDL_DestroyTexture(core_texture);
	
	if (linear_filtering) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	}
	
	core_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_RGB332,
				    SDL_TEXTUREACCESS_STREAMING,
				    MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_SetTextureBlendMode(core_texture, SDL_BLENDMODE_NONE);
}

void host_t::create_debugger_texture(bool linear_filtering)
{
	if (debugger_texture) SDL_DestroyTexture(debugger_texture);
	
	if (linear_filtering) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	}

	debugger_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_RGB332,
				    SDL_TEXTUREACCESS_STREAMING,
				    MAX_PIXELS_PER_SCANLINE, MAX_SCANLINES);
	SDL_SetTextureBlendMode(debugger_texture, SDL_BLENDMODE_NONE);
}

void host_t::create_scanlines_texture(bool linear_filtering)
{
	uint16_t *scanline_buffer = new uint16_t[4 * MAX_PIXELS_PER_SCANLINE * MAX_SCANLINES];
	
	if (scanlines_texture) SDL_DestroyTexture(scanlines_texture);

	if (linear_filtering) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	}
	
	scanlines_texture = SDL_CreateTexture(video_renderer, SDL_PIXELFORMAT_ARGB4444,
				    SDL_TEXTUREACCESS_STATIC,
				    MAX_PIXELS_PER_SCANLINE, 4 * MAX_SCANLINES);
	
	SDL_SetTextureBlendMode(scanlines_texture, SDL_BLENDMODE_BLEND);
	
	for (int i=0; i < 4*MAX_SCANLINES; i++) {
		for (int j=0; j < MAX_PIXELS_PER_SCANLINE; j++) {
			uint16_t color = 0x0000;
			if ((i % 4) == 0 || (i % 4) == 3) {
				color = 0xf000;
			}
			scanline_buffer[(i * MAX_PIXELS_PER_SCANLINE) + j] = color;
		}
	}
	
	SDL_UpdateTexture(scanlines_texture, NULL, scanline_buffer, MAX_PIXELS_PER_SCANLINE * sizeof(uint16_t));
	
	delete [] scanline_buffer;
}

enum events_output_state host_t::events_process_events()
{
	enum events_output_state return_value = NO_EVENT;
	
	SDL_Event event;
	
	//bool shift_pressed = E64_sdl2_keyboard_state[SDL_SCANCODE_LSHIFT] | E64_sdl2_keyboard_state[SDL_SCANCODE_RSHIFT];
	bool alt_pressed = sdl_keyboard_state[SDL_SCANCODE_LALT] | sdl_keyboard_state[SDL_SCANCODE_RALT];
	//bool gui_pressed   = E64_sdl2_keyboard_state[SDL_SCANCODE_LGUI] | E64_sdl2_keyboard_state[SDL_SCANCODE_RGUI];
	
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				return_value = KEYPRESS_EVENT;
				if ((event.key.keysym.sym == SDLK_f) && alt_pressed) {
					events_wait_until_key_released(SDLK_f);
					video_toggle_fullscreen();
//				} else if (event.key.keysym.sym == SDLK_F1) {
//					if (app->current_mode == DEBUG_MODE) {
//						app->core->run(0);
//						app->debugger->status();
//					}
				} else if ((event.key.keysym.sym == SDLK_s) && alt_pressed ) {
					video_change_scanlines_intensity();
				} else if ((event.key.keysym.sym == SDLK_b) && alt_pressed) {
					video_toggle_linear_filtering();
//				} else if ((event.key.keysym.sym == SDLK_r) && alt_pressed) {
//					events_wait_until_key_released(SDLK_r);
//					hud->show_notification("\nResetting system");
				} else if( (event.key.keysym.sym == SDLK_q) && alt_pressed ) {
					events_wait_until_key_released(SDLK_q);
					return_value = QUIT_EVENT;
				} else if ((event.key.keysym.sym == SDLK_MINUS) && alt_pressed) {
					// decrease screen size
					events_wait_until_key_released(SDLK_MINUS);
					video_decrease_window_size();
				} else if ((event.key.keysym.sym == SDLK_EQUALS) && alt_pressed) {
					// increase screen size
					events_wait_until_key_released(SDLK_EQUALS);
					video_increase_window_size();
				} else if(event.key.keysym.sym == SDLK_F9) {
					events_wait_until_key_released(SDLK_F9);
					app->switch_mode();
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

void host_t::video_toggle_fullscreen()
{
	video_fullscreen = !video_fullscreen;
	if (video_fullscreen) {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
		SDL_SetWindowFullscreen(video_window, SDL_WINDOW_RESIZABLE);
	}
}

void host_t::video_change_scanlines_intensity()
{
	if (video_scanlines_alpha < 64) {
		video_scanlines_alpha = 64;
	} else if (video_scanlines_alpha < 128) {
		video_scanlines_alpha = 128;
	} else if (video_scanlines_alpha < 192) {
		video_scanlines_alpha = 192;
	} else if (video_scanlines_alpha < 255) {
		video_scanlines_alpha = 255;
	} else {
		video_scanlines_alpha = 0;
	}
}

void host_t::video_toggle_linear_filtering()
{
	video_linear_filtering = !video_linear_filtering;
	create_core_texture(video_linear_filtering);
	create_debugger_texture(video_linear_filtering);
}

void host_t::video_increase_window_size()
{
	if (!video_fullscreen) {
		if (video_scaling < video_scaling_max) video_scaling++;
		SDL_SetWindowSize(video_window, video_scaling * MAX_PIXELS_PER_SCANLINE, video_scaling * ((10 * MAX_SCANLINES) / 9));
		SDL_GetWindowSize(video_window, &window_width, &window_height);
		SDL_SetWindowPosition(video_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
}

void host_t::video_decrease_window_size()
{
	if (!video_fullscreen) {
		if (video_scaling > 1) video_scaling--;
		SDL_SetWindowSize(video_window, video_scaling * MAX_PIXELS_PER_SCANLINE, video_scaling * ((10 * MAX_SCANLINES) / 9));
		SDL_GetWindowSize(video_window, &window_width, &window_height);
		SDL_SetWindowPosition(video_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
}
