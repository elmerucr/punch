/*
 * main.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "system.hpp"

int main(int argc, char **argv)
{
	system_t *system = new system_t();
	system->run();
	delete system;
	return 0;
}
