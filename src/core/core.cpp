#include "common.hpp"
#include "core.hpp"

core_t::core_t()
{
	blitter = new blitter_ic();
	mmu = new mmu_t(blitter);
	cpu = new cpu_t(mmu);
	
	blitter->vram[0xfffe] = 0x02;
	blitter->vram[0xffff] = 0x00;
	
	blitter->vram[0x0200] = 0x86;	// lda #$21
	blitter->vram[0x0201] = 0x21;
	blitter->vram[0x0202] = 0xb7;	// sta $40
	blitter->vram[0x0203] = 0x00;
	blitter->vram[0x0204] = 0x40;
	blitter->vram[0x0205] = 0x7c;	// inc $40
	blitter->vram[0x0206] = 0x00;
	blitter->vram[0x0207] = 0x40;
	blitter->vram[0x0208] = 0x7e;	// jmp $0205
	blitter->vram[0x0209] = 0x02;
	blitter->vram[0x020a] = 0x05;
}

core_t::~core_t()
{
	delete cpu;
	delete mmu;
	delete blitter;
}

void core_t::reset()
{
	cpu->reset();
}

void core_t::run(uint32_t cycles)
{
	cpu->execute();
}

void core_t::run_blitter()
{
	blitter->clear_surface(&blitter->framebuffer);
	blitter->blit(&blitter->turn_text, &blitter->framebuffer);
	blitter->blit(&blitter->bruce, &blitter->framebuffer);
	blitter->blit(&blitter->punch, &blitter->framebuffer);
}
