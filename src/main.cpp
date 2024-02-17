/*
 * main.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "system.hpp"

int main(int argc, char **argv)
{
//	for (int i = 0; i < argc; i++) {
//		printf("\n%s", argv[i]);
//	}
	
	system_t *system = new system_t();
	system->run();
	delete system;
	return 0;
}

// then m view vram and colors
// m view vram as text
