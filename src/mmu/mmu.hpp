#ifndef MMU_HPP
#define MMU_HPP

#include <cstdint>
#include "blitter.hpp"

void mmu_init(blitter_ic *b);

uint8_t mmu_read8(uint16_t address);
void mmu_write8(uint16_t address, uint8_t value);

#endif
