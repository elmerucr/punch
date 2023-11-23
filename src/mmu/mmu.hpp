#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include <cstdio>
#include "blitter.hpp"

class mmu_t {
public:
	mmu_t(blitter_ic *b) {
		blitter = b;
	}
	
	uint8_t read8(uint16_t address) {
		printf("%04x: %02x\n", address, blitter->vram[address]);
		return blitter->vram[address];
	}
	
	void write8(uint16_t address, uint8_t value) {
		blitter->vram[address] = value;
	}
private:
	blitter_ic *blitter;
};

#endif
