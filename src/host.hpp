/*
 * host.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef HOST_HPP
#define HOST_HPP

#include <SDL2/SDL.h>
#include "common.hpp"
#include "system.hpp"

enum events_output_state {
	QUIT_EVENT = -1,
	NO_EVENT = 0,
	KEYPRESS_EVENT = 1
};

enum scancodes {
	SCANCODE_EMPTY = 0x00,     // 0x00
	SCANCODE_ESCAPE,
	SCANCODE_F1,
	SCANCODE_F2,
	SCANCODE_F3,
	SCANCODE_F4,
	SCANCODE_F5,
	SCANCODE_F6,
	SCANCODE_F7,               // 0x08
	SCANCODE_F8,
	SCANCODE_GRAVE,
	SCANCODE_1,
	SCANCODE_2,
	SCANCODE_3,
	SCANCODE_4,
	SCANCODE_5,
	SCANCODE_6,                // 0x10
	SCANCODE_7,
	SCANCODE_8,
	SCANCODE_9,
	SCANCODE_0,
	SCANCODE_MINUS,
	SCANCODE_EQUALS,
	SCANCODE_BACKSPACE,
	SCANCODE_TAB,              // 0x18
	SCANCODE_Q,
	SCANCODE_W,
	SCANCODE_E,
	SCANCODE_R,
	SCANCODE_T,
	SCANCODE_Y,
	SCANCODE_U,
	SCANCODE_I,               // 0x20
	SCANCODE_O,
	SCANCODE_P,
	SCANCODE_LEFTBRACKET,
	SCANCODE_RIGHTBRACKET,
	SCANCODE_RETURN,
	SCANCODE_A,
	SCANCODE_S,
	SCANCODE_D,                // 0x28
	SCANCODE_F,
	SCANCODE_G,
	SCANCODE_H,
	SCANCODE_J,
	SCANCODE_K,
	SCANCODE_L,
	SCANCODE_SEMICOLON,
	SCANCODE_APOSTROPHE,       // 0x30
	SCANCODE_BACKSLASH,
	SCANCODE_LSHIFT,
	SCANCODE_Z,
	SCANCODE_X,
	SCANCODE_C,
	SCANCODE_V,
	SCANCODE_B,
	SCANCODE_N,                // 0x38
	SCANCODE_M,
	SCANCODE_COMMA,
	SCANCODE_PERIOD,
	SCANCODE_SLASH,
	SCANCODE_RSHIFT,
	SCANCODE_LCTRL,
	//SCANCODE_LALT,
	SCANCODE_SPACE,
	//SCANCODE_RALT,
	SCANCODE_RCTRL,              // 0x40
	SCANCODE_LEFT,
	SCANCODE_UP,
	SCANCODE_DOWN,
	SCANCODE_RIGHT
};

class host_t {
private:
	/*
	 * Audio related
	 */
	SDL_AudioDeviceID audio_device;
	SDL_AudioSpec audio_spec_want;
	SDL_AudioSpec audio_spec_have;
	double audio_bytes_per_ms;
	uint8_t audio_bytes_per_sample;
	bool audio_running{false};
	void audio_init();
	void audio_start();
	void audio_stop();

	/*
	 * Video related
	 */
	uint8_t video_scaling_max;
	uint8_t video_scaling;
	uint8_t video_scanlines_alpha{96};	// 0, 32, 64, 96, 128, 160, 192, 224, 255
	uint8_t video_hor_blur{1};
	bool shadowmask_active{false};

	bool video_fullscreen{false};
	bool video_linear_filtering{false};
	SDL_Window *video_window;
	SDL_Renderer *video_renderer;
	bool vsync;
	SDL_Texture *core_texture{nullptr};
	SDL_Texture *debugger_texture{nullptr};
	SDL_Texture *scanlines_texture{nullptr};

	SDL_Texture *shadowmask_texture{nullptr};

	int window_width;
	int window_height;

	void create_core_texture(bool linear_filtering);
	void create_debugger_texture(bool linear_filtering);
	void create_scanlines_texture(bool linear_filtering);
	void create_shadowmask_texture();

	void video_init();
	void video_stop();

	/*
	 * events related
	 */
	const uint8_t *sdl_keyboard_state;

	char current_dir[512];
	char *home;

public:
	host_t(system_t *s);
	~host_t();

	system_t *system;

	char *sdl_preference_path;

//	/*
//	 * Audio related
//	 */
	inline void queue_audio(void *buffer, unsigned size) { SDL_QueueAudio(audio_device, buffer, size); }
	inline unsigned int get_queued_audio_size_bytes() { return SDL_GetQueuedAudioSize(audio_device); }
//
	/*
	 * Video related
	 */
	void update_core_texture(uint16_t *core);
	void update_debugger_texture(uint16_t *debugger);
	void update_screen();
//	void update_title();
	void video_increase_window_size();
	void video_decrease_window_size();
	void video_toggle_fullscreen();
	void video_change_scanlines_intensity();
	void video_change_hor_blur();
	void video_toggle_shadowmask();
	void video_toggle_linear_filtering();
//
//	// getters setters
//	uint16_t current_window_width() { return video_window_sizes[current_window_size].x; }
//	uint16_t current_window_height() { return video_window_sizes[current_window_size].y; }
	inline bool vsync_enabled() { return vsync; }
	inline bool vsync_disabled() { return !vsync; }
	inline uint8_t get_bytes_per_sample() { return audio_bytes_per_sample; }
	inline double get_bytes_per_ms() { return audio_bytes_per_ms; }
//
//	//inline uint8_t get_scanlines_alpha() { return video_scanlines_alpha; }
//	//inline void set_scanline_alpha(uint8_t a) { video_scanlines_alpha = a; }
//	//inline bool is_using_vm_linear_filtering() { return linear_filtering; }
//	//inline bool is_using_hud_linear_filtering() { return hud_linear_filtering; }
//	inline bool is_using_scanlines_linear_filtering() { return scanlines_linear_filtering; }
//	//inline bool is_fullscreen() { return fullscreen; }
//	inline void set_hud(hud_t *h) { hud = h; }

	/*
	 * Events related
	 */
	enum events_output_state events_process_events();
	uint8_t keyboard_state[128];
	void events_wait_until_key_released(SDL_KeyCode key);
	bool events_yes_no();	// return true on 'y' and false on 'n'
};

#endif
