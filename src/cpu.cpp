#include "cpu.hpp"
#include "core.hpp"

uint8_t cpu_t::read8(uint16_t address) const {
	return app->core->read8(address);
}

void cpu_t::write8(uint16_t address, uint8_t value) const {
	app->core->write8(address, value);
}
