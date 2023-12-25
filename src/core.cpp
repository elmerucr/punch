#include "common.hpp"
#include "core.hpp"

extern uint8_t rom[];

core_t::core_t(app_t *a)
{
	app = a;
	
	blitter = new blitter_ic();

	/* set up framebuffer surface */
	blitter->surface[7].base = FRAMEBUFFER;
	blitter->surface[7].x = 0;
	blitter->surface[7].y = 0;
	blitter->surface[7].w = MAX_PIXELS_PER_SCANLINE;
	blitter->surface[7].h = MAX_SCANLINES;
	blitter->surface[7].bg_col = 0b00000101;

	cpu = new cpu_t(app);
	
	exceptions = new exceptions_ic();
	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);
	
	timer = new timer_ic(exceptions);
	
	sound = new sound_ic(app);
}

core_t::~core_t()
{
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

uint32_t core_t::run(int32_t cycles)
{
	uint32_t cycles_original = cycles;
	do {
		uint8_t c = cpu->execute();
		timer->run(c);
		sound->run(c);
		cycles -= c;
	} while ((cycles > 0) && !(cpu->breakpoint()));
	
	return cycles_original - cycles;
}

void core_t::run_blitter()
{
	blitter->clear_surface(7);
	blitter->blit(&blitter->turn_text, &blitter->surface[7]);
}
