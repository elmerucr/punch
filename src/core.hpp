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
#include "commander.hpp"

#define BLITTER_SURFACES		0x04
#define BLITTER_COLOR_TABLES	0x05
#define BLITTER_PALETTE			0x06 // + 0x07
#define CORE_PAGE				0x08
#define KEYBOARD_PAGE			0x09
#define	TIMER_PAGE				0x0a
#define COMMANDER_PAGE			0x0b
#define SOUND_PAGE				0x0c // + 0x0d
#define	BLITTER_PAGE			0x0e
#define VRAM_PEEK_PAGE			0x0f
#define	ROM_PAGE				0xfc

enum output_states {
	NORMAL,
	BREAKPOINT,
};

class core_t {
private:
	int32_t cpu_cycle_saldo{0};
	uint32_t sound_cycle_saldo;

	uint8_t irq_number;

	bool irq_line_frame_done{true};
	bool irq_line_load_bin{true};
	bool irq_line_load_squirrel{true};

	bool generate_interrupts_frame_done{false};
	bool generate_interrupts_load_bin{false};
	bool generate_interrupts_load_squirrel{false};

	uint32_t framebuffer_base_address{0};
	uint32_t vram_peek{0};
public:
	core_t(system_t *s);
	~core_t();

	system_t *system;

	void reset();

	int32_t get_cpu_cycle_saldo() { return cpu_cycle_saldo; }
	uint32_t get_sound_cycle_saldo() {
		uint32_t result = sound_cycle_saldo;
		sound_cycle_saldo = 0;
		return result;
	}

	enum output_states run(bool debug);

	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);

	blitter_ic *blitter;
	exceptions_ic *exceptions;
	timer_ic *timer;
	sound_ic *sound;
	cpu_t *cpu;
	clocks *cpu2sid;
	commander_t *commander;

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);

	uint32_t get_framebuffer_base_address() { return framebuffer_base_address; }

	void load_bin();
	void load_squirrel(const char *p);
};

#endif
