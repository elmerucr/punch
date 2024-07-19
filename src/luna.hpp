/*
 * luna.hpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#ifndef LUNA_HPP
#define LUNA_HPP

#include "system.hpp"

#include "lua.hpp"

#include <cstdint>

class luna_t {
private:
	system_t *system;
	lua_State *L{nullptr};
public:
	luna_t(system_t *s);
	~luna_t();
	
	void reset();
	
	bool load(const char *p);
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
};

#endif
