#ifndef BLITTER_HPP
#define BLITTER_HPP

#include <cstdint>

class blitter_ic {
public:
	blitter_ic();
	~blitter_ic();
	
	uint8_t *ram;
};

#endif
