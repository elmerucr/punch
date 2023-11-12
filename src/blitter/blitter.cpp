#include "blitter.hpp"

blitter_ic::blitter_ic()
{
	ram = new uint8_t[0x10000];
}

blitter_ic::~blitter_ic()
{
	delete [] ram;
}
