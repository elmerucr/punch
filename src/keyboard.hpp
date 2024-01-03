/*
 * keyboard.hpp
 * punch
 *
 * Copyright © 2023 elmerucr. All rights reserved.
 */

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <cstdint>
#include "system.hpp"
#include "common.hpp"
#include "host.hpp"

class keyboard_t {
private:
	//host_t *host;
	
	bool generate_events{false};
	
	bool key_down{false};
	
	int32_t microseconds_per_frame = 1000000 / FPS;
	int32_t microseconds_remaining{0};
	int32_t time_to_next{0};
	int32_t repeat_delay_ms; // in milliseconds
	int32_t repeat_speed_ms; // in milliseconds
	
	uint8_t last_char;
	
	// implement a fifo event list, important for key presses, you don't want them in the wrong order
	uint8_t event_list[256];
	void push_event(uint8_t event);	// 'head' always points to the currently available location for an event
	// if (head == tail), no events are available
	uint8_t head;
	uint8_t tail;
public:
	keyboard_t(system_t *s);
	
	system_t *system;
	
	void reset();
	void process();
	
	uint8_t pop_event();
	
	inline bool events_waiting() { return (head == tail) ? false : true; }
	inline void enable_events() { generate_events = true; }
	inline void disable_events() { generate_events = false; }
};

#endif
