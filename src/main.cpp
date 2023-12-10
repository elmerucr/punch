/*
 * main.cpp
 * punch
 *
 * Copyright © 2023 elmerucr. All rights reserved.
 */

#include "app.hpp"

int main(int argc, char **argv)
{
	app_t *app = new app_t();
	app->run();
	delete app;
	return 0;
}

// TODO: F1 in debug mode: remove garbage when pressing
//
