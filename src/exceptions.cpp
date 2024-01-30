/*
 * exceptions.cpp
 * punch / E64
 *
 * Copyright © 2019-2023 elmerucr. All rights reserved.
 */

#include "exceptions.hpp"

exceptions_ic::exceptions_ic()
{	
	for(int i=0; i<8; i++) {
		irq_input_pins[i] = true;
		name[i] = empty_name.c_str();
	}
	next_available_device = 0;
	nmi_output_pin = true;
	update_status();
}

uint8_t exceptions_ic::connect_device(const char *n)
{
	uint8_t return_value = next_available_device;
	name[next_available_device] = n;
	next_available_device++;
	next_available_device &= 7;
	return return_value;
}

void exceptions_ic::pull(uint8_t device)
{
	irq_input_pins[device & 0x7] = false;
	update_status();
}

void exceptions_ic::release(uint8_t device)
{
	irq_input_pins[device & 0x7] = true;
	update_status();
}

void exceptions_ic::toggle(uint8_t device)
{
	irq_input_pins[device & 0x7] = !irq_input_pins[device & 0x7];
	update_status();
}

void exceptions_ic::status(char *b, int buffer_length)
{
	b += snprintf(b, buffer_length, "IRQ State  Name");
	for (int i=0; i<next_available_device; i++) {
		b += snprintf(b, buffer_length, "\n %1i    %c   \"%s\"", i, irq_input_pins[i] ? '1' : '0', name[i]);
	}
}


void exceptions_ic::status(char *b, int buffer_length, uint8_t device)
{
	if (device < next_available_device) {
		snprintf(b, buffer_length, "%1i  %c \"%s\"", device, irq_input_pins[device] ? '1' : '0', name[device]);
	} else {
		b[0] = '\0';
	}
}
