/*
 * app.hpp
 * punch
 *
 * Copyright © 2023 elmerucr. All rights reserved.
 */

#ifndef APP_HPP
#define APP_HPP

#include <chrono>
#include <thread>

enum mode {
	DEBUG_MODE = 0,
	RUN_MODE = 1
};

class host_t;
class core_t;
class keyboard_t;
class debugger_t;
class stats_t;

class app_t {
private:
	std::chrono::time_point<std::chrono::steady_clock> app_start_time;
	std::chrono::time_point<std::chrono::steady_clock> end_of_frame_time;
public:
	app_t();
	~app_t();
	
	host_t *host;
	core_t *core;
	keyboard_t *keyboard;
	debugger_t *debugger;
	stats_t *stats;
	
	enum mode current_mode;
	void switch_mode();
	void switch_to_debug_mode();
	void switch_to_run_mode();
	
	void run();
	
	bool running;
};

#endif
