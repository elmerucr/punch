/*
 * system.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "system.hpp"
#include "host.hpp"
#include "core.hpp"
#include "keyboard.hpp"
#include "debugger.hpp"
#include "stats.hpp"

system_t::system_t()
{
	system_start_time = std::chrono::steady_clock::now();
	
	printf("punch v%i.%i.%i (C)%i elmerucr\n",
	       PUNCH_MAJOR_VERSION,
	       PUNCH_MINOR_VERSION,
	       PUNCH_BUILD, PUNCH_YEAR);
	
	host = new host_t(this);
	
	core = new core_t(this);
	
//	// TODO: reset blitter?
//	core->reset();
	
	keyboard = new keyboard_t(this);
	keyboard->reset();
	keyboard->enable_events();
	
	debugger = new debugger_t(this);
	
	stats = new stats_t(this);
	
	/*
	 * Default start mode
	 */
//	switch_to_run_mode();
	switch_to_debug_mode();
	
	core->reset();
}

system_t::~system_t()
{
	delete stats;
	delete debugger;
	delete keyboard;
	delete core;
	delete host;
	
	printf("[punch] %.2f seconds running time\n", (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - system_start_time).count() / 1000);
}

void system_t::switch_mode()
{
	keyboard->purge();
	if (current_mode == RUN_MODE) {
		debugger->status();
		debugger->terminal->printf(", break at $%04x", core->cpu->get_pc());
		switch_to_debug_mode();
	} else {
		switch_to_run_mode();
	}
}

void system_t::switch_to_debug_mode()
{
	debugger->prompt();
	debugger->terminal->activate_cursor();
	current_mode = DEBUG_MODE;
}

void system_t::switch_to_run_mode()
{
	debugger->terminal->deactivate_cursor();
	current_mode = RUN_MODE;
}

void system_t::run()
{
	running = true;
	
	stats->reset();
	
	end_of_frame_time = std::chrono::steady_clock::now();
	
	while (running) {
		/*
		 * Audio: Measure audio_buffer and determine audio_cycles to run
		 */
		uint32_t audio_buffer_bytes = host->get_queued_audio_size_bytes();
		stats->set_queued_audio_ms(audio_buffer_bytes / host->get_bytes_per_ms());
		
		int32_t audio_cycles = SID_CYCLES_PER_FRAME;
		
		if (audio_buffer_bytes > (AUDIO_BUFFER_SIZE * 1.2)) {
			audio_cycles -= SID_CYCLES_PER_FRAME / 100;
		} else if (audio_buffer_bytes < (AUDIO_BUFFER_SIZE * 0.8)) {
			audio_cycles += SID_CYCLES_PER_FRAME / 100;
		}
		
		core->cpu2sid->adjust_target_clock(audio_cycles);
		
		if (host->events_process_events() == QUIT_EVENT) running = false;
		
		keyboard->process();
		
		switch (current_mode) {
			case RUN_MODE:
				if (core->run(false) == BREAKPOINT) {
					switch_mode();
				}
				break;
			case DEBUG_MODE:
				debugger->run();
				debugger->redraw();
				debugger->blitter->update_framebuffer();
				host->update_debugger_texture(debugger->blitter->framebuffer);
				break;
		}
		
		uint32_t sound_cycle_saldo = core->get_sound_cycle_saldo();
		if (sound_cycle_saldo < audio_cycles ) {
			core->sound->run(audio_cycles - sound_cycle_saldo);
		}
		
		core->blitter->update_framebuffer();
		
		host->update_core_texture(core->blitter->framebuffer);
		
		//printf("%s", stats->summary());
		
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
