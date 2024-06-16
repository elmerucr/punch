/*
 * scriptor.hpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#ifndef SCRIPTOR_HPP
#define SCRIPTOR_HPP

#include "system.hpp"

#include "lua.hpp"
#include "lualib.h"
#include "lauxlib.h"

#include <cstdint>

class scriptor_t {
private:
	system_t *system;
	lua_State *L{nullptr};
public:
	scriptor_t(system_t *s);
	~scriptor_t();
	
	void reset();
	
	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
};

#endif
