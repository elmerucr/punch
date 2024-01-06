/*
 * clocks.hpp
 * punch
 *
 * Copyright © 2019-2024 elmerucr. All rights reserved.
 *
 * Algorithm based on bresenham line algorithm.
 */

#ifndef CLOCKS_HPP
#define CLOCKS_HPP

#include <cstdint>

class clocks {
private:
	uint64_t base_clock_freq;
	uint64_t target_clock_freq;
	//uint64_t clock0_cycles;
	//uint64_t clock1_cycles;

	uint64_t mult;
	uint64_t mod;
	uint64_t result;
public:
	clocks(uint32_t base_clock_f, uint32_t target_clock_f) {
		base_clock_freq = base_clock_f;
		target_clock_freq = target_clock_f;
		//clock0_cycles = 0;
		//clock1_cycles = 0;
		mod = 0;
	}
    
	inline uint32_t clock(uint32_t delta_base_clock) {
		mult = (delta_base_clock * target_clock_freq) + mod;
		
		/*
		 * Calculate new modulo
		 */
		mod  = mult % base_clock_freq;
		
		result = mult / base_clock_freq;
		//clock0_cycles += delta_clock0;
		//clock1_cycles += result;
		return (uint32_t)result;
	}
	
	inline void adjust_base_clock(uint32_t base_clock_f) {
		base_clock_freq = base_clock_f;
	}
	
	inline void adjust_target_clock(uint32_t target_clock_f) {
		target_clock_freq = target_clock_f;
	}
};

#endif
