/*
 * sound.cpp
 * punch
 *
 * Copyright © 2019-2024 elmerucr. All rights reserved.
 */

#include "sound.hpp"
#include "common.hpp"
#include "host.hpp"

/*
 * The following table is based on a SID clock frequency of 985248Hz
 * (PAL). Calculations were made according to Codebase64 article:
 * https://codebase64.org/doku.php?id=base:how_to_calculate_your_own_sid_frequency_table
 */

//const uint16_t music_notes[256] = {
//	0x008b, 0x0093, 0x009c, 0x00a6, 0x00af, 0x00ba, 0x00c5, 0x00d1, 0x00dd, 0x00ea, 0x00f8, 0x0107,	// N_C-1 (0)  to N B-1 (11)
//	0x0116, 0x0127, 0x0139, 0x014b, 0x015f, 0x0174, 0x018a, 0x01a1, 0x01ba, 0x01d4, 0x01f0, 0x020e,	// N_C0_ (12) to N_B0_ (23)
//	0x022d, 0x024e, 0x0271, 0x0296, 0x02be, 0x02e7, 0x0314, 0x0342, 0x0374, 0x03a9, 0x03e0, 0x041b,	// N_C1_ (24) to N_B1_ (35) ($02be = 28 = E1 = kick drum)
//	0x045a, 0x049c, 0x04e2, 0x052d, 0x057b, 0x05cf, 0x0627, 0x0685, 0x06e8, 0x0751, 0x07c1, 0x0837,	// N_C2_ (36) to N_B2_ (47)
//	0x08b4, 0x0938, 0x09c4, 0x0a59, 0x0af7, 0x0b9d, 0x0c4e, 0x0d0a, 0x0dd0, 0x0ea2, 0x0f81, 0x106d,	// N_C3_ (48) to N_B3_ (59)
//	0x1167, 0x1270, 0x1389, 0x14b2, 0x15ed, 0x173b, 0x189c, 0x1a13, 0x1ba0, 0x1d45, 0x1f02, 0x20da,	// N_C4_ (60) to N_B4_ (71) ($1d45 = 69 = A4 = 440Hz std)
//	0x22ce, 0x24e0, 0x2711, 0x2964, 0x2bda, 0x2e76, 0x3139, 0x3426, 0x3740, 0x3a89, 0x3e04, 0x41b4,	// N_C5_ (72) to N_B5_ (83)
//	0x459c, 0x49c0, 0x4e23, 0x52c8, 0x57b4, 0x5ceb, 0x6272, 0x684c, 0x6e80, 0x7512, 0x7c08, 0x8368,	// N_C6_ (84) to N_B6_ (95)
//	0x8b39, 0x9380, 0x9c45, 0xa590, 0xaf68, 0xb9d6, 0xc4e3, 0xd099, 0xdd00, 0xea24, 0xf810,			// N_C7_ (96) to N_A7S (106)
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
//	0,0,0,0,0,0,0,0
//};

sound_ic::sound_ic(system_t *s) : analog0(0), analog1(1), analog2(2), analog3(3)
{
	system = s;
	/*
	 * Remapping SID registers, rewiring necessary to have big endian
	 * support and even addresses for word access.
	 */

	/*
	 * voice 1
	 */
	register_index[0x00] = 0x01;    // frequency high byte
	register_index[0x01] = 0x00;    // frequency low byte
	register_index[0x02] = 0x03;    // pulsewidth high byte
	register_index[0x03] = 0x02;    // pulsewidth low byte
	register_index[0x04] = 0x04;    // control register
	register_index[0x05] = 0x05;    // attack decay
	register_index[0x06] = 0x06;    // sustain release

	register_index[0x07] = 0x1f;    // PADDING BYTE

	/*
	 * voice 2
	 */
	register_index[0x08] = 0x08;    // frequency high byte
	register_index[0x09] = 0x07;    // frequency low byte
	register_index[0x0a] = 0x0a;    // pulsewidth high byte
	register_index[0x0b] = 0x09;    // pulsewidth low byte
	register_index[0x0c] = 0x0b;    // control register
	register_index[0x0d] = 0x0c;    // attack decay
	register_index[0x0e] = 0x0d;    // sustain release

	register_index[0x0f] = 0x1f;    // PADDING BYTE

	/*
	 * voice 3
	 */
	register_index[0x10] = 0x0f;    // frequency high byte
	register_index[0x11] = 0x0e;    // frequency low byte
	register_index[0x12] = 0x11;    // pulsewidth high byte
	register_index[0x13] = 0x10;    // pulsewidth low byte
	register_index[0x14] = 0x12;    // control register
	register_index[0x15] = 0x13;    // attack decay
	register_index[0x16] = 0x14;    // sustain release

	register_index[0x17] = 0x1f;    // PADDING BYTE

	// filter
	register_index[0x18] = 0x15;    // filter cutoff low byte  (bits 0-2)
	register_index[0x19] = 0x16;    // filter cutoff high byte (bits 3-10)
	register_index[0x1a] = 0x17;    // res filt
	register_index[0x1b] = 0x18;    // filtermode / volume

	// misc
	register_index[0x1c] = 0x19;    // pot x
	register_index[0x1d] = 0x1a;    // pot y
	register_index[0x1e] = 0x1b;    // osc3_random
	register_index[0x1f] = 0x1c;    // env3

	for (int i = 0; i<4; i++) {
		/*
		 * set chip model
		 */
		sid[i].set_chip_model(MOS6581);

		/*
		 * In order to make SID produce the same pitch as it
		 * would in a PAL C64, we must use the exact same clock
		 * speed (985248Hz). We'll have the same frequencies,
		 * envelope, etc... as in the real thing.
		 *
		 * Using a sort of Bresenham algorithm it will be
		 * possible to "connect" the clock of SID to any other
		 * clock. One condition: the other one, usually the cpu
		 * clock, must be faster.
		 */
		sid[i].set_sampling_parameters(SID_CLOCK_SPEED, SAMPLE_FAST, SAMPLE_RATE);
		sid[i].enable_filter(true);
		sid[i].reset();
	}

	/*
	 * reset cycle counters for sid chips
	 */
	delta_t_sid0 = 0;
	delta_t_sid1 = 0;
	delta_t_sid2 = 0;
	delta_t_sid3 = 0;

	/*
	 * silence all balance registers
	 */
	for (int i=0; i<0x10; i++) {
		balance_registers[i] = 0x00;
	}
}

sound_ic::~sound_ic()
{
	// nothing
}

uint8_t sound_ic::io_read_byte(uint16_t address)
{
	switch (address & 0x100) {
		case 0x000:
			// sids
			switch (address & 0x1c) {
				case 0x1c:
					switch (address & 0x60) {
						case 0x00:
							return sid[0].read(register_index[address & 0x1f]);
						case 0x20:
							return sid[1].read(register_index[address & 0x1f]);
						case 0x40:
							return sid[2].read(register_index[address & 0x1f]);
						case 0x60:
							return sid[3].read(register_index[address & 0x1f]);
						default:
							return 0x00;
					}
				default:
					switch (address & 0xe0) {
						case 0x00:
							return sid[0].read(register_index[address & 0x1f]);
						case 0x20:
							return sid[1].read(register_index[address & 0x1f]);
						case 0x40:
							return sid[2].read(register_index[address & 0x1f]);
						case 0x60:
							return sid[3].read(register_index[address & 0x1f]);
						case 0x80:
						case 0xa0:
						case 0xc0:
						case 0xe0:
							return sid_shadow[address & 0x7f];
						default:
							return 0x00;
					}
			}
		case 0x100:
			// analogs + balance/mixing
			switch (address & 0xf0) {
				case 0x00:
				case 0x10:
					return analog0.read_byte(address & 0x1f);
				case 0x20:
				case 0x30:
					return analog1.read_byte(address & 0x1f);
				case 0x40:
				case 0x50:
					return analog2.read_byte(address & 0x1f);
				case 0x60:
				case 0x70:
					return analog3.read_byte(address & 0x1f);
				case 0x80:
					switch (address & 0x0f) {
						case 0x08:
							return
								(delay[0].active ? 0b00000001 : 0b0) |
								(delay[1].active ? 0b00000010 : 0b0) |
								(delay[2].active ? 0b00000100 : 0b0) |
								(delay[3].active ? 0b00001000 : 0b0) |
								(delay[4].active ? 0b00010000 : 0b0) |
								(delay[5].active ? 0b00100000 : 0b0) |
								(delay[6].active ? 0b01000000 : 0b0) |
								(delay[7].active ? 0b10000000 : 0b0) ;
						default:
							return 0x00;
					}
				case 0x90:
					return balance_registers[address & 0x0f];
				default:
					return 0x00;
			}
		default:
			return 0x00;
	}
}

void sound_ic::io_write_byte(uint16_t address, uint8_t byte)
{
	switch (address & 0x100) {
		case 0x000:
			// sids
			switch (address & 0x60) {
				case 0x00:
					sid[0].write(register_index[address & 0x1f], byte);
					break;
				case 0x20:
					sid[1].write(register_index[address & 0x1f], byte);
					break;
				case 0x40:
					sid[2].write(register_index[address & 0x1f], byte);
					break;
				case 0x60:
					sid[3].write(register_index[address & 0x1f], byte);
					break;
				default:
					break;
			}
			sid_shadow[address & 0x7f] = byte;
			break;
		case 0x100:
			// analogs + balance/mixing
			switch (address & 0xf0) {
				case 0x00:
				case 0x10:
					analog0.write_byte(address & 0x1f, byte);
					break;
				case 0x20:
				case 0x30:
					analog1.write_byte(address & 0x1f, byte);
					break;
				case 0x40:
				case 0x50:
					analog2.write_byte(address & 0x1f, byte);
					break;
				case 0x60:
				case 0x70:
					analog3.write_byte(address & 0x1f, byte);
					break;
				case 0x80:
					switch (address & 0x0f) {
						case 0x08:
							delay[0].active = (byte & 0b00000001) ? true : false;
							delay[1].active = (byte & 0b00000010) ? true : false;
							delay[2].active = (byte & 0b00000100) ? true : false;
							delay[3].active = (byte & 0b00001000) ? true : false;
							delay[4].active = (byte & 0b00010000) ? true : false;
							delay[5].active = (byte & 0b00100000) ? true : false;
							delay[6].active = (byte & 0b01000000) ? true : false;
							delay[7].active = (byte & 0b10000000) ? true : false;
							break;
						default:
							break;
					}
					break;
				case 0x90:
					balance_registers[address & 0x0f] = byte;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

void sound_ic::run(uint32_t number_of_cycles)
{
	delta_t_sid0 += number_of_cycles;
	delta_t_sid1 = delta_t_sid0;
	delta_t_sid2 = delta_t_sid0;
	delta_t_sid3 = delta_t_sid0;
	/*
	 * clock(delta_t, buf, maxNoOfSamples) function:
	 *
	 * This function returns the number of samples written by the SID chip.
	 * delta_t is a REFERENCE to the number of cycles to be processed
	 * buf is the memory area in which data should be written
	 * maxNoOfSamples (internal size of the presented buffer)
	 */
	int n = sid[0].clock(delta_t_sid0, sample_buffer_mono_sid0, 65536);
	sid[1].clock(delta_t_sid1, sample_buffer_mono_sid1, 65536);
	sid[2].clock(delta_t_sid2, sample_buffer_mono_sid2, 65536);
	sid[3].clock(delta_t_sid3, sample_buffer_mono_sid3, 65536);

	/*
	 * Analog is not connected to the cycles made by the machine,
	 * it only needs to know the amount of samples to produce.
	 */
	analog0.run(n, sample_buffer_mono_analog0);
	analog1.run(n, sample_buffer_mono_analog1);
	analog2.run(n, sample_buffer_mono_analog2);
	analog3.run(n, sample_buffer_mono_analog3);

	for (int i=0; i<n; i++) {
		f_sample_buffer_mono_sid0[i] = delay[0].sample(sample_buffer_mono_sid0[i]);
		f_sample_buffer_mono_sid1[i] = delay[1].sample(sample_buffer_mono_sid1[i]);
		f_sample_buffer_mono_sid2[i] = delay[2].sample(sample_buffer_mono_sid2[i]);
		f_sample_buffer_mono_sid3[i] = delay[3].sample(sample_buffer_mono_sid3[i]);

		f_sample_buffer_mono_analog0[i] = delay[4].sample(sample_buffer_mono_analog0[i]);
		f_sample_buffer_mono_analog1[i] = delay[5].sample(sample_buffer_mono_analog1[i]);
		f_sample_buffer_mono_analog2[i] = delay[6].sample(sample_buffer_mono_analog2[i]);
		f_sample_buffer_mono_analog3[i] = delay[7].sample(sample_buffer_mono_analog3[i]);

		// left channel
		sample_buffer_stereo[(2 * i) + 0] =
			(f_sample_buffer_mono_sid0[i]    * balance_registers[0x0]) +
			(f_sample_buffer_mono_sid1[i]    * balance_registers[0x2]) +
			(f_sample_buffer_mono_sid2[i]    * balance_registers[0x4]) +
			(f_sample_buffer_mono_sid3[i]    * balance_registers[0x6]) +
			(f_sample_buffer_mono_analog0[i] * balance_registers[0x8]) +
			(f_sample_buffer_mono_analog1[i] * balance_registers[0xa]) +
			(f_sample_buffer_mono_analog2[i] * balance_registers[0xc]) +
			(f_sample_buffer_mono_analog3[i] * balance_registers[0xe]);

		// right channel
		sample_buffer_stereo[(2 * i) + 1] =
			(f_sample_buffer_mono_sid0[i]    * balance_registers[0x1]) +
			(f_sample_buffer_mono_sid1[i]    * balance_registers[0x3]) +
			(f_sample_buffer_mono_sid2[i]    * balance_registers[0x5]) +
			(f_sample_buffer_mono_sid3[i]    * balance_registers[0x7]) +
			(f_sample_buffer_mono_analog0[i] * balance_registers[0x9]) +
			(f_sample_buffer_mono_analog1[i] * balance_registers[0xb]) +
			(f_sample_buffer_mono_analog2[i] * balance_registers[0xd]) +
			(f_sample_buffer_mono_analog3[i] * balance_registers[0xf]);

		/*
		 * Normalize both channels
		 *
		 * Output of both sids and analogs is int16_t (-32768,32767).
		 * Balance registers are 0-255. So if divided by 32768 * 255,
		 * maximum (float) output ranges from -4.0 to 4.0. That should
		 * be allright.
		 */
		sample_buffer_stereo[(2 * i) + 0] /= 32768 * 255;	// left
		sample_buffer_stereo[(2 * i) + 1] /= 32768 * 255;	// right

		if (sound_starting) {
			sample_buffer_stereo[2 * i] *= (float)(4000-sound_starting) / 4000;
			sample_buffer_stereo[(2 * i) + 1] *= (float)(4000-sound_starting) / 4000;
			sound_starting--;
		}

		//settings->audio_record_push_sample(sample_buffer_stereo[ 2 * i     ]);
		//settings->audio_record_push_sample(sample_buffer_stereo[(2 * i) + 1]);
	}

	system->host->queue_audio((void *)sample_buffer_stereo, 2 * n * system->host->get_bytes_per_sample());
}

void sound_ic::reset()
{
	sid[0].reset();
	sid[1].reset();
	sid[2].reset();
	sid[3].reset();

	sound_starting = 4000;
}
