/*
 * timer.cpp
 * punch / E64
 *
 * Copyright © 2019-2023 elmerucr. All rights reserved.
 */

#include "timer.hpp"
#include "common.hpp"
#include <cstdio>

timer_ic::timer_ic(exceptions_ic *unit)
{
	exceptions = unit;
	irq_number = exceptions->connect_device("timer");
}

void timer_ic::reset()
{
	/*
	 * No pending irq's
	 */
	status_register = 0x00;
	
	/*
	 * All timers turned off
	 */
	control_register = 0x00;
	
	for (int i=0; i<8; i++) {
		timers[i].bpm = 0x0001; // load with 1, may never be zero
		timers[i].clock_interval = bpm_to_clock_interval(timers[i].bpm);
		timers[i].counter = 0;
	}
	
	exceptions->release(irq_number);
}

void timer_ic::run(uint32_t number_of_cycles)
{
	for (int i=0; i<8; i++) {
		timers[i].counter += number_of_cycles;
		if ((timers[i].counter >= timers[i].clock_interval) && (control_register & (0b1 << i))) {
			while (timers[i].counter >= timers[i].clock_interval)
				timers[i].counter -= timers[i].clock_interval;
			exceptions->pull(irq_number);
			status_register |= (0b1 << i);
		}
	}
}

uint32_t timer_ic::bpm_to_clock_interval(uint16_t bpm)
{
	return (60.0 / bpm) * CPU_CLOCK_SPEED;
}

uint8_t timer_ic::io_read_byte(uint8_t address)
{
	switch (address & 0x1f) {
		case 0x00:
			return status_register;
		case 0x01:
			return control_register;
		case 0x10:
			return (timers[0].bpm & 0xff00) >> 8;
		case 0x11:
			return timers[0].bpm & 0xff;
		case 0x12:
			return (timers[1].bpm & 0xff00) >> 8;
		case 0x13:
			return timers[1].bpm & 0xff;
		case 0x14:
			return (timers[2].bpm & 0xff00) >> 8;
		case 0x15:
			return timers[2].bpm & 0xff;
		case 0x16:
			return (timers[3].bpm & 0xff00) >> 8;
		case 0x17:
			return timers[3].bpm & 0xff;
		case 0x18:
			return (timers[4].bpm & 0xff00) >> 8;
		case 0x19:
			return timers[4].bpm & 0xff;
		case 0x1a:
			return (timers[5].bpm & 0xff00) >> 8;
		case 0x1b:
			return timers[5].bpm & 0xff;
		case 0x1c:
			return (timers[6].bpm & 0xff00) >> 8;
		case 0x1d:
			return timers[6].bpm & 0xff;
		case 0x1e:
			return (timers[7].bpm & 0xff00) >> 8;
		case 0x1f:
			return timers[7].bpm & 0xff;
		default:
			return 0;
	}
}

void timer_ic::io_write_byte(uint8_t address, uint8_t byte)
{
	switch (address & 0x1f) {
		case 0x00:
			/*
			 *  b s   r
			 *  0 0 = 0
			 *  0 1 = 1
			 *  1 0 = 0
			 *  1 1 = 0
			 *
			 *  b = bit that's written
			 *  s = status (on if an interrupt was caused)
			 *  r = boolean result (acknowledge an interrupt (s=1) if b=1
			 *  r = (~b) & s
			 */
			status_register = (~byte) & status_register;
			if ((status_register & 0xff) == 0) {
				// no timers left causing interrupts
				exceptions->release(irq_number);
			}
			break;
		case 0x01:
		{
			uint8_t turned_on = byte & (~control_register);
			for (int i=0; i<8; i++) {
				if (turned_on & (0b1 << i)) {
					timers[i].counter = 0;
				}
			}
			control_register = byte;
			break;
		}
		case 0x10:
			timers[0].bpm = (timers[0].bpm & 0x00ff) | (byte << 8);
			if (timers[0].bpm == 0) timers[0].bpm = 1;
			timers[0].clock_interval = bpm_to_clock_interval(timers[0].bpm);
			break;
		case 0x11:
			timers[0].bpm = (timers[0].bpm & 0xff00) | byte;
			if (timers[0].bpm == 0) timers[0].bpm = 1;
			timers[0].clock_interval = bpm_to_clock_interval(timers[0].bpm);
			break;
		case 0x12:
			timers[1].bpm = (timers[1].bpm & 0x00ff) | (byte << 8);
			if (timers[1].bpm == 0) timers[0].bpm = 1;
			timers[1].clock_interval = bpm_to_clock_interval(timers[1].bpm);
			break;
		case 0x13:
			timers[1].bpm = (timers[1].bpm & 0xff00) | byte;
			if (timers[1].bpm == 0) timers[0].bpm = 1;
			timers[1].clock_interval = bpm_to_clock_interval(timers[1].bpm);
			break;
		case 0x14:
			timers[2].bpm = (timers[2].bpm & 0x00ff) | (byte << 8);
			if (timers[2].bpm == 0) timers[0].bpm = 1;
			timers[2].clock_interval = bpm_to_clock_interval(timers[2].bpm);
			break;
		case 0x15:
			timers[2].bpm = (timers[2].bpm & 0xff00) | byte;
			if (timers[2].bpm == 0) timers[0].bpm = 1;
			timers[2].clock_interval = bpm_to_clock_interval(timers[2].bpm);
			break;
		case 0x16:
			timers[3].bpm = (timers[3].bpm & 0x00ff) | (byte << 8);
			if (timers[3].bpm == 0) timers[0].bpm = 1;
			timers[3].clock_interval = bpm_to_clock_interval(timers[3].bpm);
			break;
		case 0x17:
			timers[3].bpm = (timers[3].bpm & 0xff00) | byte;
			if (timers[3].bpm == 0) timers[0].bpm = 1;
			timers[3].clock_interval = bpm_to_clock_interval(timers[3].bpm);
			break;
		case 0x18:
			timers[4].bpm = (timers[4].bpm & 0x00ff) | (byte << 8);
			if (timers[4].bpm == 0) timers[0].bpm = 1;
			timers[4].clock_interval = bpm_to_clock_interval(timers[4].bpm);
			break;
		case 0x19:
			timers[4].bpm = (timers[4].bpm & 0xff00) | byte;
			if (timers[4].bpm == 0) timers[0].bpm = 1;
			timers[4].clock_interval = bpm_to_clock_interval(timers[4].bpm);
			break;
		case 0x1a:
			timers[5].bpm = (timers[5].bpm & 0x00ff) | (byte << 8);
			if (timers[5].bpm == 0) timers[0].bpm = 1;
			timers[5].clock_interval = bpm_to_clock_interval(timers[5].bpm);
			break;
		case 0x1b:
			timers[5].bpm = (timers[5].bpm & 0xff00) | byte;
			if (timers[5].bpm == 0) timers[0].bpm = 1;
			timers[5].clock_interval = bpm_to_clock_interval(timers[5].bpm);
			break;
		case 0x1c:
			timers[6].bpm = (timers[6].bpm & 0x00ff) | (byte << 8);
			if (timers[6].bpm == 0) timers[0].bpm = 1;
			timers[6].clock_interval = bpm_to_clock_interval(timers[6].bpm);
			break;
		case 0x1d:
			timers[6].bpm = (timers[6].bpm & 0xff00) | byte;
			if (timers[6].bpm == 0) timers[0].bpm = 1;
			timers[6].clock_interval = bpm_to_clock_interval(timers[6].bpm);
			break;
		case 0x1e:
			timers[7].bpm = (timers[7].bpm & 0x00ff) | (byte << 8);
			if (timers[7].bpm == 0) timers[0].bpm = 1;
			timers[7].clock_interval = bpm_to_clock_interval(timers[7].bpm);
			break;
		case 0x1f:
			timers[7].bpm = (timers[7].bpm & 0xff00) | byte;
			if (timers[7].bpm == 0) timers[0].bpm = 1;
			timers[7].clock_interval = bpm_to_clock_interval(timers[7].bpm);
			break;
		default:
			// do nothing
			break;
	}
}

uint64_t timer_ic::get_timer_counter(uint8_t timer_number)
{
	return timers[timer_number & 0x07].counter;
}

uint64_t timer_ic::get_timer_clock_interval(uint8_t timer_number)
{
	return timers[timer_number & 0x07].clock_interval;
}

void timer_ic::set(uint8_t timer_no, uint16_t bpm)
{
	timer_no &= 0b111; // limit to max 7

	if (bpm == 0) bpm = 1;
	timers[timer_no].bpm = bpm;
	timers[timer_no].clock_interval = bpm_to_clock_interval(bpm);
	
	uint8_t byte = io_read_byte(0x01);
	io_write_byte(0x01, (0b1 << timer_no) | byte);
}

void timer_ic::status(char *buffer, uint8_t timer_no)
{
	timer_no &= 0b00000111;
	
	snprintf(buffer, 64, "\n%2x:%s/%5u/%8x/%8x",
		 timer_no,
		 control_register & (0b1 << timer_no) ? " on" : "off",
		 timers[timer_no].bpm,
		 timers[timer_no].counter,
		 timers[timer_no].clock_interval);
}
