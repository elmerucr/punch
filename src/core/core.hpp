#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>
#include "blitter.hpp"
#include "mc6809.hpp"
#include "mmu.hpp"

class cpu_t : public mc6809 {
public:
	cpu_t(mmu_t *m) {
		mmu = m;
	}
	
	uint8_t read8(uint16_t address) const {
		return mmu->read8(address);
	}
	
	void write8(uint16_t address, uint8_t value) const {
		mmu->write8(address, value);
	}
	
private:
	mmu_t *mmu;
};

class core_t {
public:
	core_t();
	~core_t();
	
	void reset();
	void run(uint32_t cycles);
private:
	blitter_ic *blitter;
	mmu_t *mmu;
	mc6809 *cpu;
};

#endif
