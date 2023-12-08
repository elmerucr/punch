#ifndef CORE_HPP
#define CORE_HPP

#include <cstdint>
#include "app.hpp"
#include "blitter.hpp"
#include "mc6809.hpp"
#include "exceptions.hpp"
#include "timer.hpp"

#define	BLITTER_PAGE	0x04
#define	TIMER_PAGE	0x05

class core_t {
public:
	core_t(app_t *a);
	~core_t();
	
	app_t *app;
	
	void reset();
	void run(int32_t cycles);
	void run_blitter();
	
	uint8_t read8(uint16_t address);
	void write8(uint16_t address, uint8_t value);
	
	blitter_ic *blitter;
	exceptions_ic *exceptions;
	timer_ic *timer;
	mc6809 *cpu;
};

class cpu_t : public mc6809 {
public:
	cpu_t(core_t *c) {
		core = c;
	}
	
	uint8_t read8(uint16_t address) const {
		return core->read8(address);
	}
	
	void write8(uint16_t address, uint8_t value) const {
		core->write8(address, value);
	}
	
private:
	core_t *core;
};

#endif
