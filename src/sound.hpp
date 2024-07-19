/*
 * sound.hpp
 * punch
 *
 * Copyright © 2019-2024 elmerucr. All rights reserved.
 */

#include <cstdio>
#include <cstdint>

#ifndef SOUND_HPP
#define SOUND_HPP

#include "sid.h" // resid header
#include "analog.hpp"
#include "system.hpp"

#include "common.hpp"

/*
 * TODO: Write description of how dealing with shadow registers: they're always written to!
 */

class digital_delay_t {
	/*
	 * What's the max? E.g. 1s (1000ms)
	 * @48000 sample rate, buffer max of 48000
	 */
	uint16_t delay_ms{200};
	uint16_t current_buffer_size;
	
	float decay{0.6};
	float dry{1.0};
	float wet{0.8};
	
	float delay_buffer[65536] = { 0 };
	uint16_t buffer_pointer{0};
	
public:
	digital_delay_t() {
		current_buffer_size = (SAMPLE_RATE / 1000) * delay_ms;
	}
	
	bool active{false};
	
	inline float sample(float input) {
		if (!active) {
			return input;
		} else {
			float output = decay * delay_buffer[buffer_pointer];
			delay_buffer[buffer_pointer] = input + output;
			buffer_pointer++;
			if (buffer_pointer >= current_buffer_size)
				buffer_pointer = 0;
			return (dry * input) + (wet * output);
		}
	}
};

class sound_ic {
private:
	/*
	 * sid variables etc...
	 */
	cycle_count delta_t_sid0;
	int16_t sample_buffer_mono_sid0[65536] = { 0 };	// this buffer connects to sid library, must be int16_t
	float f_sample_buffer_mono_sid0[65536] = { 0.0 }; // this buffer is used for processing (no clipping with floats)
	
	cycle_count delta_t_sid1;
	int16_t sample_buffer_mono_sid1[65536] = { 0 };
	float f_sample_buffer_mono_sid1[65536] = { 0.0 }; // this buffer is used for processing (no clipping with floats)
	
	cycle_count delta_t_sid2;
	int16_t sample_buffer_mono_sid2[65536] = { 0 };
	float f_sample_buffer_mono_sid2[65536] = { 0.0 }; // this buffer is used for processing (no clipping with floats)
	
	cycle_count delta_t_sid3;
	int16_t sample_buffer_mono_sid3[65536] = { 0 };
	float f_sample_buffer_mono_sid3[65536] = { 0.0 }; // this buffer is used for processing (no clipping with floats)
	
	uint8_t sid_shadow[128];
	
	/*
	 * Used to rewire several lo/hi registers from sid to big endian
	 * and make it more convenient.
	 */
	uint8_t register_index[32];

	/*
	 * Analog
	 */
	analog_ic analog0;
	analog_ic analog1;
	analog_ic analog2;
	analog_ic analog3;
	int16_t sample_buffer_mono_analog0[65536] = { 0 };
	float f_sample_buffer_mono_analog0[65536] = { 0.0 };
	int16_t sample_buffer_mono_analog1[65536];
	float f_sample_buffer_mono_analog1[65536] = { 0.0 };
	int16_t sample_buffer_mono_analog2[65536];
	float f_sample_buffer_mono_analog2[65536] = { 0.0 };
	int16_t sample_buffer_mono_analog3[65536];
	float f_sample_buffer_mono_analog3[65536] = { 0.0 };
	
	/*
	 * General
	 */
	uint8_t balance_registers[0x10];
	float sample_buffer_stereo[131072];
	
	uint16_t sound_starting;
	
	system_t *system;
public:
	sound_ic(system_t *s);
	~sound_ic();
	
	SID sid[4];
	digital_delay_t delay[8];
	
	// read and write functions to data registers of sid array and mixer
	uint8_t io_read_byte(uint16_t address);
	void io_write_byte(uint16_t address, uint8_t byte);
	// run the no of cycles that need to be processed by the sid chips on the sound device
	// and process all the accumulated cycles (flush into soundbuffer)
	void run(uint32_t number_of_cycles);
	void reset();
};

#endif
