/*
 * core.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "common.hpp"
#include "core.hpp"

extern uint8_t rom[];

core_t::core_t(system_t *s)
{
	system = s;
	
	blitter = new blitter_ic();

	/*
	 * Set up framebuffer surface.
	 * TODO: Could be done from rom as well...
	 */
	blitter->surface[7].base = FRAMEBUFFER;
	blitter->surface[7].x = 0;
	blitter->surface[7].y = 0;
	blitter->surface[7].w = MAX_PIXELS_PER_SCANLINE;
	blitter->surface[7].h = MAX_SCANLINES;
	blitter->surface[7].bg_col = 0b00000101;

	cpu = new cpu_t(system);
	
	exceptions = new exceptions_ic();
	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);
	
	timer = new timer_ic(exceptions);
	
	sound = new sound_ic(system);
	
	cpu2sid = new clocks(CPU_CLOCK_MULTIPLY, 1);
}

core_t::~core_t()
{
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
		case BLITTER_PAGE:
		case BLITTER_PAGE+1:
			return blitter->io_read8(address & 0x1ff);
		case BLITTER_PAGE+2:
		case BLITTER_PAGE+3:
			return blitter->io_palette_read8(address);
		case SYSTEM_PAGE:
			return system->io_read8(address);
		case TIMER_PAGE:
			return timer->io_read_byte(address & 0xff);
		case SOUND_PAGE:
		case SOUND_PAGE+1:
		case SOUND_PAGE+2:
		case SOUND_PAGE+3:
			return sound->io_read_byte(address & 0x3ff);
		case ROM_PAGE:
		case ROM_PAGE+1:
			return rom[address & 0x1ff];
		default:
			return blitter->vram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value) {
	uint8_t page = (address & 0xff00) >> 8;

	switch (page) {
		case BLITTER_PAGE:
		case BLITTER_PAGE+1:
			blitter->io_write8(address & 0x1ff, value);
			break;
		case BLITTER_PAGE+2:
		case BLITTER_PAGE+3:
			blitter->io_palette_write8(address, value);
			break;
		case SYSTEM_PAGE:
			system->io_write8(address, value);
			break;
		case TIMER_PAGE:
			timer->io_write_byte(address &0xff, value);
			break;
		case SOUND_PAGE:
		case SOUND_PAGE+1:
		case SOUND_PAGE+2:
		case SOUND_PAGE+3:
			sound->io_write_byte(address & 0x3ff, value);
			break;
		default:
			blitter->vram[address] = value;
			break;
	}
}

void core_t::reset()
{
	sound->reset();
	timer->reset();
	cpu->reset();
}

enum output_states core_t::run(int32_t cycles, int32_t *cycles_done)
{
	enum output_states output_state = NORMAL;
	
	*cycles_done = 0;
	
	do {
		uint8_t cpu_cycles = cpu->execute();
		uint8_t sid_cycles = cpu2sid->clock(cpu_cycles);
		
		timer->run(sid_cycles);
		sound->run(sid_cycles);
		*cycles_done += sid_cycles;
		
	} while (((cycles - *cycles_done) > 0) && !(cpu->breakpoint()));
	
	if (cpu->breakpoint()) output_state = BREAKPOINT;
	
	return output_state;
}

void core_t::run_blitter()
{
	blitter->clear_surface(7);
	blitter->blit(&blitter->turn_text, &blitter->surface[7]);
}
