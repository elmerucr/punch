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
	
	blitter->vram[0xfffe] = 0x02;
	blitter->vram[0xffff] = 0x00;
	
	blitter->vram[0x0200] = 0x86;	// lda #$21
	blitter->vram[0x0201] = 0x21;
	blitter->vram[0x0202] = 0xb7;	// sta $0400
	blitter->vram[0x0203] = 0x04;
	blitter->vram[0x0204] = 0x00;
	blitter->vram[0x0205] = 0x7c;	// inc $0400
	blitter->vram[0x0206] = 0x04;
	blitter->vram[0x0207] = 0x00;
	blitter->vram[0x0208] = 0x7e;	// jmp $0205
	blitter->vram[0x0209] = 0x02;
	blitter->vram[0x020a] = 0x05;
}

core_t::~core_t()
{
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
		       default:
			       blitter->vram[address] = value;
			       break;
	       }
       }

void core_t::reset()
{
	timer->reset();
	cpu->reset();
}

void core_t::run(int32_t cycles)
{
	 do {
		cycles -= cpu->execute();
	 } while (cycles > 0);
}

void core_t::run_blitter()
{
	blitter->clear_surface(&blitter->framebuffer);
	blitter->blit(&blitter->turn_text, &blitter->framebuffer);
	blitter->blit(&blitter->bruce, &blitter->framebuffer);
	blitter->blit(&blitter->punch, &blitter->framebuffer);
}

uint8_t cpu_t::read8(uint16_t address) const {
	return app->core->read8(address);
}

void cpu_t::write8(uint16_t address, uint8_t value) const {
	app->core->write8(address, value);
}
