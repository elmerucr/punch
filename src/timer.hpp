/*
 * timer.hpp
 * punch / E64
 *
 * Copyright © 2019-2023 elmerucr. All rights reserved.
 */
 
/*
 * Register 0 - Status Register (SR)
 *
 * (READ)
 * 7 6 5 4 3 2 1 0
 * | | | | | | | |
 * | | | | | | | +-- 1 = timer0 pulled the irq
 * | | | | | | +---- 1 = timer1 pulled the irq
 * | | | | | +------ 1 = timer2 pulled the irq
 * | | | | +-------- 1 = timer3 pulled the irq
 * | | | +---------- 1 = timer4 pulled the irq
 * | | +------------ 1 = timer5 pulled the irq
 * | +-------------- 1 = timer6 pulled the irq
 * +---------------- 1 = timer7 pulled the irq
 *
 *
 * On write, interrupts will be acknowledged. Only if all bits are zero again,
 * interrupt line is up again.
 *
 */

/*
 * Register 1 - Control Register (CR)
 *
 * READ and WRITE:
 * 7 6 5 4 3 2 1 0
 *         | | | |
 *         | | | +-- timer0 interrupts, 1=on, 0=off
 *         | | +---- timer1 interrupts, 1=on, 0=off
 *         | +------ timer2 interrupts, 1=on, 0=off
 *         +-------- timer3 interrupts, 1=on, 0=off
 *                   etc...
 *
 */

/*
 * Registers 2 to 15 unused
 */


/*
 * Registers 16 to 31 are respectively the high and low bytes of
 * unsigned 16bit data values.
 */

#ifndef timer_hpp
#define timer_hpp

#include <cstdint>
#include "exceptions.hpp"

struct timer_unit {
	uint16_t bpm;
	uint32_t clock_interval;
	
	// NEEDS WORK? used to be 64 bit...
	uint32_t counter;
};

class timer_ic
{
private:
	uint8_t status_register;
	uint8_t control_register;
	
	struct timer_unit timers[8];

	uint32_t bpm_to_clock_interval(uint16_t bpm);
	
	exceptions_ic *exceptions;
public:
	timer_ic(exceptions_ic *unit);
	void reset();
	
	uint8_t irq_number;

	// register access functions
	uint8_t io_read_byte(uint8_t address);
	void io_write_byte(uint8_t address, uint8_t byte);

	// get functions
	uint64_t get_timer_counter(uint8_t timer_number);
	uint64_t get_timer_clock_interval(uint8_t timer_number);
	uint16_t get_timer_bpm(uint8_t timer_number);

	// run cycles on this ic
	void run(uint32_t number_of_cycles);
	
	// convenience function (turning on specific timer + bpm)
	void set(uint8_t timer_no, uint16_t bpm);
	
	void status(char *buffer, uint8_t timer_no);
};

#endif
