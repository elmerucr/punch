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

// basepage 00e0 not working?
// next step: tile surface base becomes base page
// then m view vram and colors
// m view vram as text
// page for color index table (so default to hardware table etc)
// then 1, 2, 4, 8 and 16 bits per pixel will become possible
// 16mb ram done
