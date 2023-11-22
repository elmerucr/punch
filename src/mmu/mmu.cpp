#include <cstdio>
#include "mmu.hpp"

blitter_ic *_blitter;

void mmu_init(blitter_ic *b)
{
	_blitter = b;
}

uint8_t mmu_read8(uint16_t address)
{
	printf("%04x: %02x\n", address, _blitter->vram[address]);
	return _blitter->vram[address];
}

void mmu_write8(uint16_t address, uint8_t value)
{
	_blitter->vram[address] = value;
}
