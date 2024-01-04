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
	
	uint8_t irq_number;
	bool irq_line{true};
	
	bool generate_interrupts{false};
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
	
	int32_t frame_cycles{0};
	int32_t frame_cycles_remaining{0};
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
};

#endif
