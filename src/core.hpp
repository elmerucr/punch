/*
 * core.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>
#include "system.hpp"
#include "blitter.hpp"
#include "cpu.hpp"
#include "exceptions.hpp"
#include "sound.hpp"
#include "timer.hpp"
#include "clocks.hpp"

// core / cia / vram inspect
#define	BLITTER_PAGE	0x04
#define SYSTEM_PAGE	0x08
#define	TIMER_PAGE	0x0a
#define SOUND_PAGE	0x0c
#define	ROM_PAGE	0xfe

enum output_states {
	NORMAL,
	BREAKPOINT,
};

class core_t {
private:
	bool frame_done{false};
	int32_t cpu_cycle_saldo{0};
	
public:
	core_t(system_t *s);
	~core_t();
	
	system_t *system;
	
	void reset();
	bool is_frame_done() { return frame_done; }
	
	enum output_states run(int32_t cycles, int32_t *cycles_done);
	enum output_states run(bool debug);
	void run_blitter();
	
	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);
	
	blitter_ic *blitter;
	exceptions_ic *exceptions;
	timer_ic *timer;
	sound_ic *sound;
	cpu_t *cpu;
	clocks *cpu2sid;
};

#endif
