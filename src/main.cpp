#include <cstdio>
#include <cstdint>
#include "common.hpp"
#include "host.hpp"
#include "keyboard.hpp"
#include "core.hpp"
#include "debugger.hpp"
#include <chrono>
#include <thread>

int main()
{
	std::chrono::time_point<std::chrono::steady_clock> app_start_time = std::chrono::steady_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> end_of_frame_time;
	
	printf("punch v%i.%i.%i (C)%i elmerucr\n",
	       PUNCH_MAJOR_VERSION,
	       PUNCH_MINOR_VERSION,
	       PUNCH_BUILD, PUNCH_YEAR);
	
	host_t *host = new host_t();
	core_t *core = new core_t();
	core->reset();
	debugger_t *debugger = new debugger_t(core);
	debugger->terminal->activate_cursor();
	
	keyboard_t *keyboard = new keyboard_t(host);
	keyboard->reset();
	keyboard->enable_events();
	
	bool running = true;
	
	int32_t cycles = 0;
	
	end_of_frame_time = std::chrono::steady_clock::now();
	
	uint8_t symbol = 0;
	
	while (running) {
		if (host->events_process_events() == QUIT_EVENT) running = false;
		
		keyboard->process();
		
		cycles += 985248;
		
		core->run(cycles);
		core->run_blitter();
		
		debugger->terminal->process_cursor_state();
		while (keyboard->events_waiting()) {
			debugger->terminal->deactivate_cursor();
			symbol = keyboard->pop_event();
			switch (symbol) {
				case ASCII_CURSOR_LEFT:
					debugger->terminal->cursor_left();
					break;
				case ASCII_CURSOR_RIGHT:
					debugger->terminal->cursor_right();
					break;
				case ASCII_CURSOR_UP:
					debugger->terminal->cursor_up();
					break;
				case ASCII_CURSOR_DOWN:
					debugger->terminal->cursor_down();
					break;
				case ASCII_BACKSPACE:
					debugger->terminal->backspace();
					break;
				case ASCII_LF:
					printf("\n%s", debugger->terminal->get_command());
					// fall through to default
				default:
					debugger->terminal->putchar(symbol);
					break;
			}
			debugger->terminal->activate_cursor();
		}
		debugger->redraw();
		
		host->update_textures(&core->blitter->vram[(core->blitter->framebuffer_bank & 0x0f) << 16], &debugger->blitter->vram[(debugger->blitter->framebuffer_bank & 0x0f) << 16]);
		
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
	}

	printf("[punch] %.2f seconds running time\n", (double)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - app_start_time).count() / 1000);
	
	delete keyboard;
	delete debugger;
	delete core;
	delete host;
	
	return 0;
}
