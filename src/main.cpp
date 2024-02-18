/*
 * main.cpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#include "system.hpp"

int main(int argc, char **argv)
{
//	PLACEHOLDER for directly starting with a rom later on...
//
//	for (int i = 0; i < argc; i++) {
//		printf("\n%s", argv[i]);
//	}
	system_t *system = new system_t();
	system->run();
	delete system;
	return 0;
}
