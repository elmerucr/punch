/*
 * system.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

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

class system_t {
private:
	std::chrono::time_point<std::chrono::steady_clock> system_start_time;
	std::chrono::time_point<std::chrono::steady_clock> end_of_frame_time;
public:
	system_t();
	~system_t();
	
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
