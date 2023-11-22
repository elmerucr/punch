#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>
#include "blitter.hpp"
#include "mc6809.hpp"
#include "mmu.hpp"

class core_t {
public:
	core_t();
	~core_t();
	
	void reset();
	void run(uint32_t cycles);
private:
	blitter_ic *blitter;
	mc6809 *cpu;
};

#endif
