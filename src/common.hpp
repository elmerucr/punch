

#define BUILD			20231125

#define FPS			60
#define MAX_PIXELS_PER_SCANLINE	320
#define MAX_SCANLINES		180
//#define MAX_PIXELS_PER_SCANLINE	320
//#define MAX_SCANLINES		180
#define PIXELS			(MAX_PIXELS_PER_SCANLINE*MAX_SCANLINES)

#define VRAM_SIZE		0x100000
#define VRAM_SIZE_MASK		(VRAM_SIZE-1)

/*
 * "sort of" C64 colors
 */
#define C64_BLACK		0b00000000
#define C64_WHITE		0b11111111
#define C64_RED			0b10101001
#define C64_CYAN		0b10011011
#define C64_PURPLE		0b01100010
#define C64_GREEN		0b00001100
#define C64_BLUE		0b00100110
#define C64_YELLOW		0b11011001
#define C64_ORANGE		0b10001101
#define C64_BROWN		0b01000100
#define C64_LIGHTRED		0b11110010
#define C64_DARKGREY		0b01001001
#define C64_GREY		0b10010010
#define C64_LIGHTGREEN		0b10010110
#define C64_LIGHTBLUE		0b10010011
#define C64_LIGHTGREY		0b11011011

/*
 * C64 colors (VirtualC64)
 */
//#define C64_BLACK       0xf000
//#define C64_WHITE       0xffff
//#define C64_RED         0xf733
//#define C64_CYAN        0xf8cc
//#define C64_PURPLE      0xf849
//#define C64_GREEN       0xf6a5
//#define C64_BLUE        0xf339
//#define C64_YELLOW      0xfee8
//#define C64_ORANGE      0xf853
//#define C64_BROWN       0xf531
//#define C64_LIGHTRED    0xfb77
//#define C64_DARKGREY    0xf444
//#define C64_GREY        0xf777
//#define C64_LIGHTGREEN  0xfbfa
//#define C64_LIGHTBLUE   0xf67d
//#define C64_LIGHTGREY   0xfaaa
