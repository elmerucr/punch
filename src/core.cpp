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

	/*
	 * Set up framebuffer surface.
	 * TODO: Could be done from rom as well...
	 */
	blitter->surface[7].base_page = FRAMEBUFFER_PAGE;
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
	
	cpu2sid = new clocks(CPU_CLOCK_SPEED/FPS, SID_CLOCK_SPEED/FPS);
	
	irq_number = exceptions->connect_device("core");
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
		case CORE_PAGE:
			return io_read8(address);
		case KEYBOARD_PAGE:
			return system->keyboard->io_read8(address);
		case TIMER_PAGE:
			return timer->io_read_byte(address & 0xff);
		case VRAM_PEEK_PAGE:
			return blitter->vram[((vram_peek << 8) | (address & 0xff)) & VRAM_SIZE_MASK];
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
		case CORE_PAGE:
			io_write8(address, value);
			break;
		case KEYBOARD_PAGE:
			system->keyboard->io_write8(address, value);
			break;
		case TIMER_PAGE:
			timer->io_write_byte(address &0xff, value);
			break;
		case VRAM_PEEK_PAGE:
			blitter->vram[((vram_peek << 8) | (address & 0xff)) & VRAM_SIZE_MASK] = value;
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
	cpu_cycle_saldo = 0;
	irq_line = true;
	
	sound->reset();
	timer->reset();
	cpu->reset();
	
	blitter->set_pixel_saldo(MAX_PIXELS_PER_FRAME);
}

enum output_states core_t::run(bool debug)
{
	enum output_states output_state = NORMAL;
	
	do {
		uint8_t cpu_cycles = cpu->execute();
		timer->run(cpu_cycles);
		uint8_t sound_cycles = cpu2sid->clock(cpu_cycles);
		sound->run(sound_cycles);
		cpu_cycle_saldo += cpu_cycles;
		sound_cycle_saldo += sound_cycles;
		
	} while ((!cpu->breakpoint()) && (cpu_cycle_saldo < CPU_CYCLES_PER_FRAME) && (!debug));
	
	if (cpu->breakpoint()) output_state = BREAKPOINT;
	
	if (cpu_cycle_saldo >= CPU_CYCLES_PER_FRAME) {
		cpu_cycle_saldo -= CPU_CYCLES_PER_FRAME;
		if (generate_interrupts) {
			exceptions->pull(irq_number);
			irq_line = false;
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
			return irq_line ? 0 : 1;
		case 0x01:
			// control register
			return generate_interrupts ? 1 : 0;
		case 0x02:
			// vram peek high byte
			return (vram_peek & 0xff00) >> 8;
		case 0x03:
			// vram peek low byte
			return vram_peek & 0x00ff;
		default:
			return 0;
	}
}

void core_t::io_write8(uint16_t address, uint8_t value)
{
	switch (address & 0x0f) {
		case 0x00:
			if ((value & 0b1) && (!irq_line)) {
				exceptions->release(irq_number);
				irq_line = true;
			}
			break;
		case 0x01:
			generate_interrupts = (value & 0b1) ? true : false;
			break;
		case 0x02:
			// vram peek high byte
			vram_peek = (vram_peek & 0x00ff) | (value << 8);
			break;
		case 0x03:
			// vram peek low byte
			vram_peek = (vram_peek & 0xff00) | value;
			break;
		default:
			break;
	}
}
