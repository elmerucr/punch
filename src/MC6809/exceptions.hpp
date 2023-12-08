/*
 * exceptions.hpp
 * punch / E64
 *
 *  Copyright © 2019-2023 elmerucr. All rights reserved.
 */

/*
 * TODO: NMI not implemented?
 */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <cstdint>

class exceptions_ic {
private:
	uint8_t next_available_device;
	
	bool irq_input_pins[8];
	
	inline void update_status() {
		bool result =	(!(irq_input_pins[0])) |
				(!(irq_input_pins[1])) |
				(!(irq_input_pins[2])) |
				(!(irq_input_pins[3])) |
				(!(irq_input_pins[4])) |
				(!(irq_input_pins[5])) |
				(!(irq_input_pins[6])) |
				(!(irq_input_pins[7])) ;
		irq_output_pin = !result;
	}
	
public:
	exceptions_ic();
	bool irq_output_pin;
	bool nmi_output_pin;
	uint8_t connect_device();
	void pull(uint8_t device);
	void release(uint8_t device);
};

#endif
