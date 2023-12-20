#include "common.hpp"
#include "core.hpp"

core_t::core_t(app_t *a)
{
	app = a;
	
	blitter = new blitter_ic();

	cpu = new cpu_t(app);
	
	exceptions = new exceptions_ic();
	cpu->assign_nmi_line(&exceptions->nmi_output_pin);
	cpu->assign_irq_line(&exceptions->irq_output_pin);
	
	timer = new timer_ic(exceptions);
	
	sound = new sound_ic(app);
	
	blitter->vram[0xfffe] = 0x02;
	blitter->vram[0xffff] = 0x00;
	
	blitter->vram[0x0200] = 0x10;	// lds #$fff0
	blitter->vram[0x0201] = 0xce;
	blitter->vram[0x0202] = 0xff;
	blitter->vram[0x0203] = 0xf0;
	blitter->vram[0x0204] = 0x1c;	// andcc #%10101111	; enable firq/irq
	blitter->vram[0x0205] = 0b10101111;
					
	blitter->vram[0x0206] = 0x86;	// lda #$21
	blitter->vram[0x0207] = 0x21;
	blitter->vram[0x0208] = 0xb7;	// sta $0400
	blitter->vram[0x0209] = 0x04;
	blitter->vram[0x020a] = 0x00;
	blitter->vram[0x020b] = 0x7c;	// inc $0400
	blitter->vram[0x020c] = 0x04;
	blitter->vram[0x020d] = 0x00;
	blitter->vram[0x020e] = 0x7e;	// jmp $020b
	blitter->vram[0x020f] = 0x02;
	blitter->vram[0x0210] = 0x0b;
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
			return blitter->io_read8(address & 0xff);
		case TIMER_PAGE:
			return timer->io_read_byte(address & 0xff);
		case SOUND_PAGE:
		case SOUND_PAGE+1:
		case SOUND_PAGE+2:
		case SOUND_PAGE+3:
			return sound->io_read_byte(address & 0x3ff);
		default:
			return blitter->vram[address];
	}
}

void core_t::write8(uint16_t address, uint8_t value) {
	uint8_t page = (address & 0xff00) >> 8;

	switch (page) {
		case BLITTER_PAGE:
			blitter->io_write8(address & 0xff, value);
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

void core_t::run(int32_t cycles)
{
	do {
		uint8_t cycles_done = cpu->execute();
		timer->run(cycles_done);
		sound->run(cycles_done);
		cycles -= cycles_done;
	} while (cycles > 0);
}

void core_t::run_blitter()
{
	blitter->clear_surface(&blitter->framebuffer);
	blitter->blit(&blitter->turn_text, &blitter->framebuffer);
	blitter->blit(&blitter->bruce, &blitter->framebuffer);
	blitter->blit(&blitter->punch, &blitter->framebuffer);
}
