/*
 * rca.hpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

#ifndef RCA_HPP
#define RCA_HPP

class rca {
private:
	/*
	 * Will be a 29 cell prng, fits perfectly in a 32 bit register
	 */
	uint32_t stat;
public:
	/*
	 * Default constructor, simple init, turn on 1 bit in the
	 * "middle column", this bit serves also as the least significant
	 * bit of the byte that's returned.
	 */
	rca() {
		/*
		 * Initial state of the 32bit register (29 bits are used):
		 *
		 * |x0000000|00000001|00000000|000000xx|
		 *
		 * One bit on the left side and two bits on the right side not
		 * used.
		 */
		stat = (1 << 16);
	}
	
	/*
	 * Alternative constructor,uses an uint32_t for init.
	 */
	rca(uint32_t number) {
		/*
		 * Alternative manual constructor
		 */
		stat = number;
	}

	/*
	 * Generates a "random byte", this function currently actually
	 * performs two tasks:
	 *
	 * (1) Shifts internal status according to rule 30 (Wolfram)
	 * (2) Outputs part of internal status as 8 bit unsigned integer
	 */
	uint8_t byte() {
		/*
		 * Clean up the left and right side of the ca, make sure
		 * things are clean before processing
		 */
		// TODO: Can't we do this by ANDing with 0b011111..111000??? Probably nicer...
		stat = (((stat<<1)>>3)<<2);

		/*
		 * Need two temporary registers to hold data and combine...
		 */
		uint32_t temp1, temp2;

		/*
		 * Make the status has periodic boundary conditions: bit 1 gets a
		 * copy of bit 30. Bit 31 (leftmost) gets a copy of bit 2.
		 */
		temp1 = stat >> 29;
		temp2 = stat << 29;
		stat = stat | temp1 | temp2;

		temp1 = stat >> 1;
		temp2 = stat << 1;

		/*
		 * The actual rule 30 from Wolfram
		 */
		stat = (stat | temp2) ^ temp1;

		return (uint8_t)((stat<<8)>>24);
	}
	
	/*
	 * Returns status (e.g. to seed another generator), doesn't
	 * apply rule 30 on internal status.
	 */
	uint32_t status() {
		return stat;
	}
};

#endif
