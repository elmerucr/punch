/*
 * core.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "common.hpp"
#include "core.hpp"
#include "keyboard.hpp"

extern uint8_t rom[];

core_t::core_t(system_t *s)
{
	system = s;

	blitter = new blitter_ic();

	cpu = new cpu_t(system);

	exceptions = new exceptions_ic();
	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);

	timer = new timer_ic(exceptions);

	sound = new sound_ic(system);

	cpu2sid = new clocks(CPU_CLOCK_SPEED/FPS, SID_CLOCK_SPEED/FPS);

	irq_number = exceptions->connect_device("core");

	/*
	 * Last one!
	 */
	commander = new commander_t(system);
}

core_t::~core_t()
{
	delete commander;
	delete cpu2sid;
	delete sound;
	delete timer;
	delete exceptions;
	delete cpu;
	delete blitter;
}

uint8_t core_t::read8(uint16_t address)
{
	uint8_t page = (address & 0xff00) >> 8;

	switch (page) {
		case CORE_PAGE:
			return io_read8(address);
		case KEYBOARD_PAGE:
			return system->keyboard->io_read8(address);
		case TIMER_PAGE:
			return timer->io_read_byte(address & 0xff);
		case COMMANDER_PAGE:
			return commander->io_read8(address & 0xff);
		case SOUND_PAGE:
		case SOUND_PAGE+1:
			return sound->io_read_byte(address & 0x1ff);
		case BLITTER_PAGE:
		case BLITTER_PAGE+1:	// vram peek
		case BLITTER_PAGE+2:	// surfaces
		case BLITTER_PAGE+3:	// color tables
		case BLITTER_PAGE+4:
		case BLITTER_PAGE+5:
		case BLITTER_PAGE+6:
		case BLITTER_PAGE+7:
			// TODO: hack
			return blitter->io_read8((address - 0xe00) & 0x7ff);
		case ROM_PAGE:
		case ROM_PAGE+1:
		case ROM_PAGE+2:
		case ROM_PAGE+3:
			return rom[address & 0x3ff];
		default:
			return blitter->vram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value) {
	uint8_t page = (address & 0xff00) >> 8;

	switch (page) {
		case CORE_PAGE:
			io_write8(address, value);
			break;
		case KEYBOARD_PAGE:
			system->keyboard->io_write8(address, value);
			break;
		case TIMER_PAGE:
			timer->io_write_byte(address & 0xff, value);
			break;
		case COMMANDER_PAGE:
			commander->io_write8(address & 0xff, value);
			break;
		case SOUND_PAGE:
		case SOUND_PAGE+1:
			sound->io_write_byte(address & 0x1ff, value);
			break;
		case BLITTER_PAGE:
		case BLITTER_PAGE+1:	// vram peek
		case BLITTER_PAGE+2:	// surfaces
		case BLITTER_PAGE+3:	// color tables
		case BLITTER_PAGE+4:
		case BLITTER_PAGE+5:
		case BLITTER_PAGE+6:
		case BLITTER_PAGE+7:
			// TODO: hack
			blitter->io_write8((address - 0xe00) & 0x7ff, value);
			break;
		default:
			blitter->vram[address] = value;
			break;
	}
}

void core_t::reset()
{
	cpu_cycle_saldo = 0;
	irq_line_frame_done = true;

	sound->reset();
	timer->reset();
	cpu->reset();
	blitter->reset();

	blitter->set_pixel_saldo(MAX_PIXELS_PER_FRAME);

	/*
	 * Default font 4x6 at surface $e
	 */
	blitter->surface[0xe].w = 4;
	blitter->surface[0xe].h = 6;
	blitter->surface[0xe].flags_0 = 0b00000010;
	blitter->surface[0xe].flags_1 = 0b00000000;
	blitter->surface[0xe].flags_2 = 0b00000001;
	// no need for base_address (implied by flags_2)

	// some little check if deadbeef looks SCRAMBLED meaning host is little endian
	uint32_t *e = (uint32_t *)&blitter->vram[0x2000];
	*e = 0xefbeadde;

	commander->reset();
}

enum output_states core_t::run(bool debug)
{
	enum output_states output_state = NORMAL;

	do {

		uint16_t cpu_cycles = cpu->execute();
		timer->run(cpu_cycles);
		uint16_t sound_cycles = cpu2sid->clock(cpu_cycles);
		sound->run(sound_cycles);
		cpu_cycle_saldo += cpu_cycles;
		sound_cycle_saldo += sound_cycles;

	} while ((!cpu->breakpoint()) && (cpu_cycle_saldo < CPU_CYCLES_PER_FRAME) && (!debug));

	if (cpu->breakpoint()) output_state = BREAKPOINT;

	if (cpu_cycle_saldo >= CPU_CYCLES_PER_FRAME) {
		cpu_cycle_saldo -= CPU_CYCLES_PER_FRAME;
		if (generate_interrupts_frame_done) {
			exceptions->pull(irq_number);
			irq_line_frame_done = false;
		}
		blitter->set_pixel_saldo(MAX_PIXELS_PER_FRAME);
	}

	return output_state;
}

uint8_t core_t::io_read8(uint16_t address)
{
	switch (address & 0x0f) {
		case 0x00:
			// status register
			return
				(irq_line_frame_done    ? 0b00000000 : 0b00000001) |
				(irq_line_load_bin      ? 0b00000000 : 0b00000010) |
				(irq_line_load_squirrel ? 0b00000000 : 0b00001000) ;
		case 0x01:
			// control register
			return
				(generate_interrupts_frame_done    ? 0b00000001 : 0b00000000) |
				(generate_interrupts_load_bin      ? 0b00000010 : 0b00000000) |
				(generate_interrupts_load_squirrel ? 0b00001000 : 0b00000000) ;
		default:
			return 0;
	}
}

void core_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x0f) {
		case 0x00:
			if ((value & 0b00000001) && (!irq_line_frame_done)) {
				irq_line_frame_done = true;
			}
			if ((value & 0b00000010) && (!irq_line_load_bin)) {
				irq_line_load_bin = true;
			}
			if ((value & 0b00001000) && (!irq_line_load_squirrel)) {
				irq_line_load_squirrel = true;
			}
			if (irq_line_frame_done && irq_line_load_bin && irq_line_load_squirrel) exceptions->release(irq_number);
			break;
		case 0x01:
			generate_interrupts_frame_done    = (value & 0b00000001) ? true : false;
			generate_interrupts_load_bin      = (value & 0b00000010) ? true : false;
			generate_interrupts_load_squirrel = (value & 0b00001000) ? true : false;
			break;
		default:
			break;
	}
}

void core_t::load_bin()
{
	if (generate_interrupts_load_bin) {
		irq_line_load_bin = false;
		exceptions->pull(irq_number);
	}
}

void core_t::load_squirrel(const char *p)
{
	if (generate_interrupts_load_squirrel) {
		if (!commander->load_squirrel(p)) {
			// loading succesful
			irq_line_load_squirrel = false;
			exceptions->pull(irq_number);
		}
	}
}
