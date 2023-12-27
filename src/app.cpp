/*
 * app.cpp
 * punch
 *
 * Copyright © 2023 elmerucr. All rights reserved.
 */

#include "app.hpp"
#include "host.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "debugger.hpp"
#include "stats.hpp"

app_t::app_t()
{
	app_start_time = std::chrono::steady_clock::now();
	
	printf("punch v%i.%i.%i (C)%i elmerucr\n",
	       PUNCH_MAJOR_VERSION,
	       PUNCH_MINOR_VERSION,
	       PUNCH_BUILD, PUNCH_YEAR);
	
	host = new host_t(this);
	
	core = new core_t(this);
	core->reset();
	
	keyboard = new keyboard_t(this);
	keyboard->reset();
	keyboard->enable_events();
	
	debugger = new debugger_t(this);
	
	stats = new stats_t(this);
	
//	switch_to_run_mode();
	switch_to_debug_mode();
}

app_t::~app_t()
{
	delete stats;
	delete debugger;
	delete keyboard;
	delete core;
	delete host;
	
	printf("[punch] %.2f seconds running time\n", (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - app_start_time).count() / 1000);
}

void app_t::switch_mode()
{
	if (current_mode == RUN_MODE) {
		debugger->status();
		debugger->terminal->printf("\n\nbreak at $%04x: switching to debug mode", core->cpu->get_pc());
		switch_to_debug_mode();
	} else {
		switch_to_run_mode();
	}
}

void app_t::switch_to_debug_mode()
{
	debugger->prompt();
	debugger->terminal->activate_cursor();
	current_mode = DEBUG_MODE;
}

void app_t::switch_to_run_mode()
{
	debugger->terminal->deactivate_cursor();
	current_mode = RUN_MODE;
}

void app_t::run()
{
	running = true;
	
	stats->reset();
	
	end_of_frame_time = std::chrono::steady_clock::now();
	
	while (running) {
		/*
		 * Audio: Measure audio_buffer and determine cycles to run
		 */
		uint32_t audio_buffer = host->get_queued_audio_size_bytes();
		stats->set_queued_audio_bytes(audio_buffer);
		
		/*
		 * Use int32_t, not uint32_t! Adjust to needed buffer size + change to cycles.
		 */
		int32_t cycles = SID_CLOCK_SPEED * (AUDIO_BUFFER_SIZE - audio_buffer) / (host->get_bytes_per_ms() * 1000);
		/*
		 * Add number of cycles needed for one frame
		 */
		cycles += SID_CLOCK_SPEED / FPS;
		
		//stats->start_core_time();
		
		if (host->events_process_events() == QUIT_EVENT) running = false;
		
		keyboard->process();
		
		uint32_t cycles_done{0};
		
		switch (current_mode) {
			case RUN_MODE:
				cycles_done = core->run(cycles);
//				if (cycles > 0) {
//					core->sound->run(cycles);
//				}
				if (core->cpu->breakpoint()) switch_mode();
				break;
			case DEBUG_MODE:
				core->sound->run(cycles);
				debugger->run();
				debugger->redraw();
				host->update_debugger_texture(&debugger->blitter->vram[FRAMEBUFFER]);
				break;
		}

		core->run_blitter(); // run always?
		host->update_core_texture(&core->blitter->vram[FRAMEBUFFER]);
		
		printf("%s", stats->summary());
		
		// Time measurement
		stats->start_idle_time();
		
		/*
		 * If vsync is enabled, the update screen function takes more
		 * time, i.e. it will return after a few milliseconds, exactly
		 * when vertical refresh is done. This will avoid tearing.
		 * There's no need then to let the system sleep with a
		 * calculated value. But we will still have to do a time
		 * measurement for estimation of idle time.
		 *
		 * When there's no vsync, sleep time is done manually.
		 */
		if (host->vsync_disabled()) {
			end_of_frame_time += std::chrono::microseconds(1000000/FPS);
			/*
			 * If the next update is in the past, calculate a
			 * new update moment.
			 */
			if (end_of_frame_time > std::chrono::steady_clock::now()) {
				std::this_thread::sleep_until(end_of_frame_time);
			} else {
				end_of_frame_time = std::chrono::steady_clock::now();
			}
		}
		
		host->update_screen();

		stats->start_core_time();
		
		stats->process_parameters();
	}
}
