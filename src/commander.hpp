/*
 * commander.hpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#ifndef COMMANDER_HPP
#define COMMANDER_HPP

#include "system.hpp"

#define _SQ64
#include "squirrel.h"
#include "sqstdaux.h"
#include "sqstdblob.h"
#include "sqstdio.h"

#include <cstdint>

class commander_t {
private:
	system_t *system;
	HSQUIRRELVM v{nullptr};
public:
	commander_t(system_t *s);
	~commander_t();

	void reset();

	bool load_squirrel(const char *p);

	uint8_t io_read8(uint16_t address);
	void io_write8(uint16_t address, uint8_t value);
};

#endif
