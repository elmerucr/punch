#ifndef CPU_HPP
#define CPU_HPP

#include "mc6809.hpp"
#include "app.hpp"

class cpu_t : public mc6809 {
public:
	cpu_t(app_t *a) {
		app = a;
	}
	
	uint8_t read8(uint16_t address) const;
	void write8(uint16_t address, uint8_t value) const;
	
private:
	app_t *app;
};

#endif
