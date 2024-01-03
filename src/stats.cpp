/*
 * stats.cpp
 * punch
 *
 * Copyright © 2020-2023 elmerucr. All rights reserved.
 */

#include <cstdint>
#include <chrono>
#include <thread>
#include <cstdint>
#include <iostream>
#include "stats.hpp"
#include "common.hpp"
#include "core.hpp"
#include "cpu.hpp"

void stats_t::reset()
{
	total_time = 0;
	total_sound_time = 0;
	total_core_time = 0;
	total_idle_time = 0;
	
	framecounter = 0;
	framecounter_interval = 4;
	
	status_bar_framecounter = 0;
	status_bar_framecounter_interval = FPS / 2;

	audio_queue_size_ms = 0;
	smoothed_audio_queue_size_ms = 0;
	
	smoothed_framerate = FPS;
	
	smoothed_cpu_mhz = CPU_CLOCK_MULTIPLY*SID_CLOCK_SPEED/(1000*1000);
	old_cpu_ticks = system->core->cpu->clock_ticks();
	
	smoothed_core_per_frame = 1000000 / (FPS * 4);
	smoothed_idle_per_frame = 1000000 / (FPS * 4);
	
	cpu_percentage = 100 * smoothed_core_per_frame / (1000000 / FPS);
    
	alpha = 0.90f;
	alpha_cpu = 0.50f;
	
	frametime = 1000000 / FPS;

	start_core = start_core_old = std::chrono::steady_clock::now();
}

void stats_t::process_parameters()
{
	framecounter++;
	
	if (framecounter == framecounter_interval) {
		framecounter = 0;

		framerate = (double)(framecounter_interval * 1000000) / total_time;
		
		smoothed_framerate =
			(alpha * smoothed_framerate) +
			((1.0 - alpha) * framerate);
		
		/*
		 * cpu speed
		 */
		cpu_ticks = system->core->cpu->clock_ticks();
		cpu_mhz = (double)(cpu_ticks - old_cpu_ticks) / total_time;
		smoothed_cpu_mhz =
			(alpha_cpu * smoothed_cpu_mhz) +
			((1.0 - alpha_cpu) * cpu_mhz);
		old_cpu_ticks = cpu_ticks;

		core_per_frame = total_core_time / framecounter_interval;
		idle_per_frame = total_idle_time / framecounter_interval;
		
		smoothed_core_per_frame =
			(alpha * smoothed_core_per_frame) +
			((1.0 - alpha) * core_per_frame);
		
		smoothed_idle_per_frame =
			(alpha * smoothed_idle_per_frame) +
			((1.0 - alpha) * idle_per_frame);
		
		smoothed_audio_queue_size_ms =
			(alpha * smoothed_audio_queue_size_ms) +
			((1.0 - alpha) * audio_queue_size_ms);
		
		cpu_percentage = 100 * smoothed_core_per_frame / (smoothed_core_per_frame + smoothed_idle_per_frame);
        
		total_time = total_sound_time = total_core_time = total_idle_time = 0;
	}

	status_bar_framecounter++;

	if (status_bar_framecounter == status_bar_framecounter_interval) {
		status_bar_framecounter = 0;

		snprintf(statistics_string, 256,
			"\n  frametime: %5.2f ms     cpu load:%6.2f %%\n"
			"       core: %5.2f ms  audiobuffer: %5.2f ms\n"
			"        cpu: %5.2f mHz   framerate:%6.2f fps\n",
			(smoothed_core_per_frame+smoothed_idle_per_frame)/1000, cpu_percentage,
			smoothed_core_per_frame/1000, smoothed_audio_queue_size_ms,
			smoothed_cpu_mhz, smoothed_framerate);
	}
}
