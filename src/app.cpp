/*
 * app.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
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
	
	bool start_of_new_frame{true};
	int32_t frame_cycles{0};
	int32_t frame_cycles_remaining{0};
	
	while (running) {
		/*
		 * Audio: Measure audio_buffer and determine cycles to run
		 */
		uint32_t audio_buffer_bytes = host->get_queued_audio_size_bytes();
		stats->set_queued_audio_ms(audio_buffer_bytes / host->get_bytes_per_ms());
		
		int32_t audio_cycles = (SID_CLOCK_SPEED / FPS);
		
		if (audio_buffer_bytes > (AUDIO_BUFFER_SIZE * 1.2)) {
			audio_cycles -= (SID_CLOCK_SPEED / FPS) / 100;
		} else if (audio_buffer_bytes < (AUDIO_BUFFER_SIZE * 0.8)) {
			audio_cycles += (SID_CLOCK_SPEED / FPS) / 100;
		}
		
		if (start_of_new_frame) {
			frame_cycles = frame_cycles_remaining = audio_cycles;
			start_of_new_frame = false;
			// raise interrupt here?
		}
		
		/*
		 * This situation is unlikely but may happen when
		 * there is a breakpoint just after the start of a frame.
		 * If this next frame is slightly shorter then adjust
		 * frame_cycles_left
		 */
		if (frame_cycles_remaining > audio_cycles) frame_cycles = frame_cycles_remaining = audio_cycles;
		
		if (host->events_process_events() == QUIT_EVENT) running = false;
		
		keyboard->process();
		
		int32_t frame_cycles_done{0};
		
		switch (current_mode) {
			case RUN_MODE:
				switch (core->run(frame_cycles_remaining, &frame_cycles_done)) {
					case NORMAL:
						// frame is finished
						// --> Have interrupt next round!
						frame_cycles_remaining -= frame_cycles_done;
						start_of_new_frame = true;
						break;
					case BREAKPOINT:
						// frame is not finished!
						switch_mode();
						core->sound->run(audio_cycles - frame_cycles_done);
						frame_cycles_remaining -= frame_cycles_done;
						printf("%i cycles left of %i\n", frame_cycles_remaining, frame_cycles);
						break;
				}
				break;
			case DEBUG_MODE:
				debugger->run(&frame_cycles_done);
				core->sound->run(audio_cycles - frame_cycles_done);
				debugger->redraw();
				debugger->blitter->update_framebuffer();
				host->update_debugger_texture(debugger->blitter->framebuffer);
				break;
		}
		core->run_blitter(); // run always?
		
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
