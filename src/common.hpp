/*
 * common.hpp
 * punch
 *
 * Copyright © 2023-2024 elmerucr. All rights reserved.
 */

#ifndef COMMON_HPP
#define COMMON_HPP

/*
 * Version information
 */
#define PUNCH_MAJOR_VERSION    0
#define PUNCH_MINOR_VERSION    7
#define PUNCH_BUILD            20240719
#define PUNCH_YEAR             2024

/*
 * Video
 */
#define VRAM_SIZE				0x1000000
#define VRAM_SIZE_MASK			(VRAM_SIZE-1)
#define FPS						60
#define MAX_PIXELS_PER_SCANLINE	320
#define MAX_SCANLINES			180
#define PIXELS					(MAX_PIXELS_PER_SCANLINE*MAX_SCANLINES)
#define MAX_PIXELS_PER_FRAME	524288	// 512k pixels = 9.1x total screen pixels
#define FRAMEBUFFER_ADDRESS		0x0e0000

/*
 * Audio
 */
#define SAMPLE_RATE				48000
#define AUDIO_BUFFER_SIZE		8000.0
#define SID_CLOCK_SPEED			985248
#define SID_CYCLES_PER_FRAME	(SID_CLOCK_SPEED/FPS)

/*
 * Cpu
 */
#define CPU_CLOCK_MULTIPLY		1
#define CPU_CLOCK_SPEED			(CPU_CLOCK_MULTIPLY*SID_CLOCK_SPEED)
#define CPU_CYCLES_PER_FRAME	(CPU_CLOCK_SPEED/FPS)

/*
 * Colors similar to C64 colors
 */
#define PUNCH_BLACK			0x00
#define PUNCH_WHITE			0xff
#define PUNCH_RED			0xc6
#define PUNCH_CYAN			0x7e
#define PUNCH_PURPLE		0x8e
#define PUNCH_GREEN			0x72
#define PUNCH_BLUE			0x1e
#define PUNCH_YELLOW		0xf7
#define PUNCH_ORANGE		0xe2
#define PUNCH_BROWN			0x91
#define PUNCH_LIGHTRED		0xea
#define PUNCH_DARKGREY		0xa9
#define PUNCH_GREY			0xab
#define PUNCH_LIGHTGREEN	0xbb
#define PUNCH_LIGHTBLUE		0xae
#define PUNCH_LIGHTGREY		0xfe

/*
 * C64 colors (VirtualC64) argb4444 format
 */
#define C64_ARGB444_BLACK       0xf000
#define C64_ARGB444_WHITE       0xffff
#define C64_ARGB444_RED         0xf733
#define C64_ARGB444_CYAN        0xf8cc
#define C64_ARGB444_PURPLE      0xf849
#define C64_ARGB444_GREEN       0xf6a5
#define C64_ARGB444_BLUE        0xf339
#define C64_ARGB444_YELLOW      0xfee8
#define C64_ARGB444_ORANGE      0xf853
#define C64_ARGB444_BROWN       0xf531
#define C64_ARGB444_LIGHTRED    0xfb77
#define C64_ARGB444_DARKGREY    0xf444
#define C64_ARGB444_GREY        0xf777
#define C64_ARGB444_LIGHTGREEN  0xfbfa
#define C64_ARGB444_LIGHTBLUE   0xf67d
#define C64_ARGB444_LIGHTGREY   0xfaaa


/*
 * Ascii values (some of them are petscii)
 */
#define ASCII_NULL          0x00    // null
#define ASCII_BACKSPACE     0x08
#define ASCII_HOR_TAB       0x09
#define ASCII_CR            0x0d    // carriage return
#define ASCII_LF            0x0a    // linefeed
#define ASCII_CURSOR_DOWN   0x11    // petscii
#define ASCII_REVERSE_ON    0x12    // petscii
#define ASCII_ESCAPE        0x1b
#define ASCII_CURSOR_RIGHT  0x1d    // petscii
#define ASCII_SPACE         0x20    // space
#define ASCII_EXCL_MARK     0x21    // !
#define ASCII_DOUBLE_QUOTES 0x22    // "
#define ASCII_NUMBER        0x23    // #
#define ASCII_DOLLAR        0x24    // $
#define ASCII_PERCENT       0x25    // %
#define ASCII_AMPERSAND     0x26    // &
#define ASCII_SINGLE_QUOTE  0x27    // '
#define ASCII_OPEN_PAR      0x28    // (
#define ASCII_CLOSE_PAR     0x29    // )
#define ASCII_ASTERISK      0x2a    // *
#define ASCII_PLUS          0x2b    // +
#define ASCII_COMMA         0x2c    // ,
#define ASCII_HYPHEN        0x2d    // -
#define ASCII_PERIOD        0x2e    // .
#define ASCII_SLASH         0x2f    // /
#define ASCII_0             0x30    // 0
#define ASCII_1             0x31    // 1
#define ASCII_2             0x32    // 2
#define ASCII_3             0x33    // 3
#define ASCII_4             0x34    // 4
#define ASCII_5             0x35    // 5
#define ASCII_6             0x36    // 6
#define ASCII_7             0x37    // 7
#define ASCII_8             0x38    // 8
#define ASCII_9             0x39    // 9
#define ASCII_COLON         0x3a    // :
#define ASCII_SEMI_COLON    0x3b    // ;
#define ASCII_LESS          0x3c    // <
#define ASCII_EQUALS        0x3d    // =
#define ASCII_GREATER       0x3e    // >
#define ASCII_QUESTION_M    0x3f    // ?
#define ASCII_AT            0x40    // @
#define ASCII_A             0x41
#define ASCII_B             0x42
#define ASCII_C             0x43
#define ASCII_D             0x44
#define ASCII_E             0x45
#define ASCII_F             0x46
#define ASCII_G             0x47
#define ASCII_H             0x48
#define ASCII_I             0x49
#define ASCII_J             0x4a
#define ASCII_K             0x4b
#define ASCII_L             0x4c
#define ASCII_M             0x4d
#define ASCII_N             0x4e
#define ASCII_O             0x4f
#define ASCII_P             0x50
#define ASCII_Q             0x51
#define ASCII_R             0x52
#define ASCII_S             0x53
#define ASCII_T             0x54
#define ASCII_U             0x55
#define ASCII_V             0x56
#define ASCII_W             0x57
#define ASCII_X             0x58
#define ASCII_Y             0x59
#define ASCII_Z             0x5a
#define ASCII_OPEN_BRACK    0x5b    // [
#define ASCII_BACKSLASH     0x5c    // \    patched
#define ASCII_CLOSE_BRACK   0x5d    // ]
#define ASCII_CARET         0x5e    // ^    patched
#define ASCII_UNDERSCORE    0x5f    // _
#define ASCII_GRAVE         0x60    // `
#define ASCII_a             0x61
#define ASCII_b             0x62
#define ASCII_c             0x63
#define ASCII_d             0x64
#define ASCII_e             0x65
#define ASCII_f             0x66
#define ASCII_g             0x67
#define ASCII_h             0x68
#define ASCII_i             0x69
#define ASCII_j             0x6a
#define ASCII_k             0x6b
#define ASCII_l             0x6c
#define ASCII_m             0x6d
#define ASCII_n             0x6e
#define ASCII_o             0x6f
#define ASCII_p             0x70
#define ASCII_q             0x71
#define ASCII_r             0x72
#define ASCII_s             0x73
#define ASCII_t             0x74
#define ASCII_u             0x75
#define ASCII_v             0x76
#define ASCII_w             0x77
#define ASCII_x             0x78
#define ASCII_y             0x79
#define ASCII_z             0x7a
#define ASCII_OPEN_BRACE    0x7b    // {
#define ASCII_VERT_BAR      0x7c    // |
#define ASCII_CLOSE_BRACE   0x7d    // }
#define ASCII_TILDE         0x7e    // ~
#define ASCII_DELETE        0x7f

#define ASCII_F1            0x85    // taken from cbm petscii
#define ASCII_F2            0x86    // taken from cbm petscii
#define ASCII_F3            0x87    // taken from cbm petscii
#define ASCII_F4            0x88    // taken from cbm petscii
#define ASCII_F5            0x89    // taken from cbm petscii
#define ASCII_F6            0x8a    // taken from cbm petscii
#define ASCII_F7            0x8b    // taken from cbm petscii
#define ASCII_F8            0x8c    // taken from cbm petscii

#define ASCII_CURSOR_UP     0x91    // petscii cursor up
#define ASCII_REVERSE_OFF   0x92    // petscii
#define ASCII_CURSOR_LEFT   0x9d    // petscii cursor left

#endif
