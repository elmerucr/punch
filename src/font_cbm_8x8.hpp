/*
 * font_cbm_8x8.hpp
 * punch
 *
 * Copyright © 2024 elmerucr. All rights reserved.
 */

/*
 * The original C64 characters are copyright Commodore Business Systems.
 * The order of characters and characters themselves have been altered to
 * match code page 437.
 *
 * 2020-10-23 elmerucr
 *
 * Part of punch - Character ROM 2kb
 */

#include <cstdint>

#ifndef FONT_CBM_8X8_HPP
#define FONT_CBM_8X8_HPP

class font_cbm_8x8_t {
public:
	uint8_t data[2048] = {
		0b00000000,		// $00 (space)
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,

		0b01111110,		// $01 (smiley)
		0b10000001,
		0b10100101,
		0b10000001,
		0b10111101,
		0b10011001,
		0b10000001,
		0b01111110,

		0b01111110,		// $02 (smiley invert)
		0b11111111,
		0b11011011,
		0b11111111,
		0b11000011,
		0b11100111,
		0b11111111,
		0b01111110,

		0b00110110,		// $03 (hearts)
		0b01111111,
		0b01111111,
		0b01111111,
		0b00111110,
		0b00011100,
		0b00001000,
		0b00000000,

		0b00001000,		// $04 (diamonds)
		0b00011100,
		0b00111110,
		0b01111111,
		0b00111110,
		0b00011100,
		0b00001000,
		0b00000000,

		0b00011000,		// $05 (clubs)
		0b00011000,
		0b01100110,
		0b01100110,
		0b00011000,
		0b00011000,
		0b00111100,
		0b00000000,

		0b00001000,		// $06 (spades)
		0b00011100,
		0b00111110,
		0b01111111,
		0b01111111,
		0b00011100,
		0b00111110,
		0b00000000,

		0b00000000,		// $07 (ball)
		0b00000000,
		0b00011000,
		0b00111100,
		0b00111100,
		0b00011000,
		0b00000000,
		0b00000000,

		0b11111111,		// $08 (reverse ball)
		0b11111111,
		0b11100111,
		0b11000011,
		0b11000011,
		0b11100111,
		0b11111111,
		0b11111111,

		0b00000000,		// $09 (circle)
		0b00111100,
		0b01100110,
		0b01000010,
		0b01000010,
		0b01100110,
		0b00111100,
		0b00000000,

		0b11111111,		// $0A (reverse circle)
		0b11000011,
		0b10011001,
		0b10111101,
		0b10111101,
		0b10011001,
		0b11000011,
		0b11111111,

		0b00001111,		// $0B
		0b00000111,
		0b00001111,
		0b01111101,
		0b11001100,
		0b11001100,
		0b11001100,
		0b01111000,

		0b00111100,		// $0C
		0b01100110,
		0b01100110,
		0b01100110,
		0b00111100,
		0b00011000,
		0b01111110,
		0b00011000,

		0b00111111,		// $0D
		0b00110011,
		0b00111111,
		0b00110000,
		0b00110000,
		0b01110000,
		0b11110000,
		0b01110000,

		0b01111111,		// $0E
		0b01100011,
		0b01111111,
		0b01100011,
		0b01100011,
		0b01100111,
		0b11100110,
		0b11000000,

		0b10011001,		// $0F
		0b01011010,
		0b00111100,
		0b11100111,
		0b11100111,
		0b00111100,
		0b01011010,
		0b10011001,

		0b01000000,		// $10
		0b01110000,
		0b01111100,
		0b01111111,
		0b01111100,
		0b01110000,
		0b01000000,
		0b00000000,

		0b00000001,		// $11
		0b00000111,
		0b00011111,
		0b01111111,
		0b00011111,
		0b00000111,
		0b00000001,
		0b00000000,

		0b00011000,		// $12
		0b00111100,
		0b01111110,
		0b00011000,
		0b00011000,
		0b01111110,
		0b00111100,
		0b00011000,

		0b01100110,		// $13
		0b01100110,
		0b01100110,
		0b01100110,
		0b01100110,
		0b00000000,
		0b01100110,
		0b00000000,

		0b01111111,		// $14
		0b11011011,
		0b11011011,
		0b01111011,
		0b00011011,
		0b00011011,
		0b00011011,
		0b00000000,

		0b00111110,		// $15
		0b01100011,
		0b00111000,
		0b01101100,
		0b01101100,
		0b00111000,
		0b11001100,
		0b01111000,

		0b00000000,		// $16
		0b00000000,
		0b00000000,
		0b00000000,
		0b01111110,
		0b01111110,
		0b01111110,
		0b00000000,

		0b00011000,		// $17
		0b00111100,
		0b01111110,
		0b00011000,
		0b01111110,
		0b00111100,
		0b00011000,
		0b11111111,

		0b00011000,		// $18 (arrow up)
		0b00111100,
		0b01111110,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000,

		0b00011000,		// $19 (arrow down)
		0b00011000,
		0b00011000,
		0b00011000,
		0b01111110,
		0b00111100,
		0b00011000,
		0b00000000,

		0b00000000,		// $1A (arrow right)
		0b00000100,
		0b00000110,
		0b01111111,
		0b01111111,
		0b00000110,
		0b00000100,
		0b00000000,

		0b00000000,		// $1B (arrow left)
		0b00010000,
		0b00110000,
		0b01111111,
		0b01111111,
		0b00110000,
		0b00010000,
		0b00000000,

		0b00000000,		// $1C
		0b00000000,
		0b01100000,
		0b01100000,
		0b01100000,
		0b01111111,
		0b00000000,
		0b00000000,

		0b00000000,		// $1D
		0b00010100,
		0b00110110,
		0b01111111,
		0b01111111,
		0b00110110,
		0b00010100,
		0b00000000,

		0b00000000,		// $1E
		0b00011000,
		0b00111100,
		0b01111110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b00000000,

		0b00000000,		// $1F
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000,
		0b00000000,

		0b00000000,		// $20 space
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,

		0b00011000,		// $21 !
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000,
		0b00000000,
		0b00011000,
		0b00000000,

		0b01100110,		// $22 "
		0b01100110,
		0b01100110,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,

		0b01100110,		// $23 #
		0b01100110,
		0b11111111,
		0b01100110,
		0b11111111,
		0b01100110,
		0b01100110,
		0b00000000,

		0b00011000,		// $24 $
		0b00111110,
		0b01100000,
		0b00111100,
		0b00000110,
		0b01111100,
		0b00011000,
		0b00000000,

	    0x62, 0x66, 0x0c, 0x18, 0x30, 0x66, 0x46, 0x00,	// $25 %
	    0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00,	// $26 &
	    0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,	// $27 '
	    0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00,	// $28 (
	    0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00,	// $29 )
	    0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00,	// $2A *
	    0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00,	// $2B +
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,	// $2C ,
	    0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,	// $2D -
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,	// $2E .
	    0x00, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x00,	// $2F /

	    0x3c, 0x66, 0x6e, 0x76, 0x66, 0x66, 0x3c, 0x00,	// $30 0
	    0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7e, 0x00,	// $31 1
	    0x3c, 0x66, 0x06, 0x0c, 0x30, 0x60, 0x7e, 0x00,	// $32 2
	    0x3c, 0x66, 0x06, 0x1c, 0x06, 0x66, 0x3c, 0x00,	// $33 3
	    0x06, 0x0e, 0x1e, 0x66, 0x7f, 0x06, 0x06, 0x00,	// $34 4
	    0x7e, 0x60, 0x7c, 0x06, 0x06, 0x66, 0x3c, 0x00,	// $35 5
	    0x3c, 0x66, 0x60, 0x7c, 0x66, 0x66, 0x3c, 0x00,	// $36 6
	    0x7e, 0x66, 0x0c, 0x18, 0x18, 0x18, 0x18, 0x00,	// $37 7
	    0x3c, 0x66, 0x66, 0x3c, 0x66, 0x66, 0x3c, 0x00,	// $38 8
	    0x3c, 0x66, 0x66, 0x3e, 0x06, 0x66, 0x3c, 0x00,	// $39 9

	    0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00,	// $3A :
	    0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,	// $3B ;
	    0x0e, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0e, 0x00,	// $3C <
	    0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00,	// $3D =
	    0x70, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x70, 0x00,	// $3E >
	    0x3c, 0x66, 0x06, 0x0c, 0x18, 0x00, 0x18, 0x00,	// $3F ?

	    0x3c, 0x66, 0x6e, 0x6e, 0x60, 0x62, 0x3c, 0x00,	// $40 @
	    0x18, 0x3c, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00,	// $41 A
	    0x7c, 0x66, 0x66, 0x7c, 0x66, 0x66, 0x7c, 0x00,	// $42 B
	    0x3c, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3c, 0x00,	// $43 C
	    0x78, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0x78, 0x00,	// $44 D
	    0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x7e, 0x00,	// $45 E
	    0x7e, 0x60, 0x60, 0x78, 0x60, 0x60, 0x60, 0x00,	// $46 F
	    0x3c, 0x66, 0x60, 0x6e, 0x66, 0x66, 0x3c, 0x00,	// $47 G
	    0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00,	// $48 H
	    0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,	// $49 I
	    0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x6c, 0x38, 0x00,	// $4A J
	    0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00,	// $4B K
	    0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7e, 0x00,	// $4C L
	    0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00,	// $4D M
	    0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00,	// $4E N
	    0x3c, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,	// $4F O
	    0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60, 0x60, 0x00,	// $50 P
	    0x3c, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x0e, 0x00,	// $51 Q
	    0x7c, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0x66, 0x00,	// $52 R
	    0x3c, 0x66, 0x60, 0x3c, 0x06, 0x66, 0x3c, 0x00,	// $53 S
	    0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,	// $54 T
	    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x00,	// $55 U
	    0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00,	// $56 V
	    0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00,	// $57 W
	    0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00,	// $58 X
		0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00,	// $59 Y
	    0x7e, 0x06, 0x0c, 0x18, 0x30, 0x60, 0x7e, 0x00,	// $5A Z

	    0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00,	// $5B [
	    0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x00,	// $5C backslash
	    0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00,	// $5D ]
	    0x18, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00,	// $5E ^
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00,	// $5F _

	    0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,	// $60 `
	    0x00, 0x00, 0x3c, 0x06, 0x3e, 0x66, 0x3e, 0x00,	// $61 a
	    0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x00,	// $62 b
	    0x00, 0x00, 0x3c, 0x60, 0x60, 0x60, 0x3c, 0x00,	// $63 c
	    0x00, 0x06, 0x06, 0x3e, 0x66, 0x66, 0x3e, 0x00,	// $64 d
	    0x00, 0x00, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00,	// $65 e
	    0x00, 0x0e, 0x18, 0x3e, 0x18, 0x18, 0x18, 0x00,	// $66 f
	    0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x7c,	// $67 g
	    0x00, 0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x00,	// $68 h
	    0x00, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00,	// $69 i
	    0x00, 0x06, 0x00, 0x06, 0x06, 0x06, 0x06, 0x3c,	// $6A j
	    0x00, 0x60, 0x60, 0x6c, 0x78, 0x6c, 0x66, 0x00,	// $6B k
	    0x00, 0x38, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,	// $6C l
	    0x00, 0x00, 0x66, 0x7f, 0x7f, 0x6b, 0x63, 0x00,	// $6D m
	    0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00,	// $6E n
	    0x00, 0x00, 0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00,	// $6F o
	    0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60,	// $70 p
	    0x00, 0x00, 0x3e, 0x66, 0x66, 0x3e, 0x06, 0x06,	// $71 q
	    0x00, 0x00, 0x7c, 0x66, 0x60, 0x60, 0x60, 0x00,	// $72 r
	    0x00, 0x00, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00,	// $73 s
	    0x00, 0x18, 0x7e, 0x18, 0x18, 0x18, 0x0e, 0x00,	// $74 t
	    0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00,	// $75 u
	    0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00,	// $76 v
	    0x00, 0x00, 0x63, 0x6b, 0x7f, 0x3e, 0x36, 0x00,	// $77 w
	    0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00,	// $78 x
	    0x00, 0x00, 0x66, 0x66, 0x66, 0x3e, 0x0c, 0x78,	// $79 y
	    0x00, 0x00, 0x7e, 0x0c, 0x18, 0x30, 0x7e, 0x00,	// $7A z

	    0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00,	// $7B {
	    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,	// $7C |
	    0x38, 0x0c, 0x0c, 0x07, 0x0c, 0x0c, 0x38, 0x00,	// $7D }
	    0x00, 0x00, 0x70, 0xdb, 0x0e, 0x00, 0x00, 0x00,	// $7E ~

		0b00000000,		// $7F
		0b00001000,
		0b00011100,
		0b00110110,
		0b01100011,
		0b01100011,
		0b01111111,
		0b00000000,

    // inverted chars
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0x81, 0x7e, 0x5a, 0x7e, 0x42, 0x66, 0x7e, 0x81,
	    0x81, 0x00, 0x24, 0x00, 0x3c, 0x18, 0x00, 0x81,
	    0xc9, 0x80, 0x80, 0x80, 0xc1, 0xe3, 0xf7, 0xff,
	    0xf7, 0xe3, 0xc1, 0x80, 0xc1, 0xe3, 0xf7, 0xff,
	    0xe7, 0xe7, 0x99, 0x99, 0xe7, 0xe7, 0xc3, 0xff,
	    0xf7, 0xe3, 0xc1, 0x80, 0x80, 0xe3, 0xc1, 0xff,
	    0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff,
	    0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00,
	    0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff,
	    0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00,
	    0xf0, 0xf8, 0xf0, 0x82, 0x33, 0x33, 0x33, 0x87,
	    0xc3, 0x99, 0x99, 0x99, 0xc3, 0xe7, 0x81, 0xe7,
	    0xc0, 0xcc, 0xc0, 0xcf, 0xcf, 0x8f, 0x0f, 0x8f,
	    0x80, 0x9c, 0x80, 0x9c, 0x9c, 0x98, 0x19, 0x3f,
	    0x66, 0xa5, 0xc3, 0x18, 0x18, 0xc3, 0xa5, 0x66,
	    0xbf, 0x8f, 0x83, 0x80, 0x83, 0x8f, 0xbf, 0xff,
	    0xfe, 0xf8, 0xe0, 0x80, 0xe0, 0xf8, 0xfe, 0xff,
	    0xe7, 0xc3, 0x81, 0xe7, 0xe7, 0x81, 0xc3, 0xe7,
	    0x99, 0x99, 0x99, 0x99, 0x99, 0xff, 0x99, 0xff,
	    0x80, 0x24, 0x24, 0x84, 0xe4, 0xe4, 0xe4, 0xff,
	    0xc1, 0x9c, 0xc7, 0x93, 0x93, 0xc7, 0x33, 0x87,
	    0xff, 0xff, 0xff, 0xff, 0x81, 0x81, 0x81, 0xff,
	    0xe7, 0xc3, 0x81, 0xe7, 0x81, 0xc3, 0xe7, 0x00,
	    0xe7, 0xc3, 0x81, 0xe7, 0xe7, 0xe7, 0xe7, 0xff,
	    0xe7, 0xe7, 0xe7, 0xe7, 0x81, 0xc3, 0xe7, 0xff,
	    0xff, 0xfb, 0xf9, 0x80, 0x80, 0xf9, 0xfb, 0xff,
	    0xff, 0xef, 0xcf, 0x80, 0x80, 0xcf, 0xef, 0xff,
	    0xff, 0xff, 0x9f, 0x9f, 0x9f, 0x80, 0xff, 0xff,
	    0xff, 0xeb, 0xc9, 0x80, 0x80, 0xc9, 0xeb, 0xff,
	    0xff, 0xe7, 0xc3, 0x81, 0x00, 0x00, 0x00, 0xff,
	    0xff, 0x00, 0x00, 0x00, 0x81, 0xc3, 0xe7, 0xff,
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0xff, 0xe7, 0xff,
	    0x99, 0x99, 0x99, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0x99, 0x99, 0x00, 0x99, 0x00, 0x99, 0x99, 0xff,
	    0xe7, 0xc1, 0x9f, 0xc3, 0xf9, 0x83, 0xe7, 0xff,
	    0x9d, 0x99, 0xf3, 0xe7, 0xcf, 0x99, 0xb9, 0xff,
	    0xc3, 0x99, 0xc3, 0xc7, 0x98, 0x99, 0xc0, 0xff,
	    0xf9, 0xf3, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0xf3, 0xe7, 0xcf, 0xcf, 0xcf, 0xe7, 0xf3, 0xff,
	    0xcf, 0xe7, 0xf3, 0xf3, 0xf3, 0xe7, 0xcf, 0xff,
	    0xff, 0x99, 0xc3, 0x00, 0xc3, 0x99, 0xff, 0xff,
	    0xff, 0xe7, 0xe7, 0x81, 0xe7, 0xe7, 0xff, 0xff,
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xe7, 0xcf,
	    0xff, 0xff, 0xff, 0x81, 0xff, 0xff, 0xff, 0xff,
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xe7, 0xe7, 0xff,
	    0xff, 0xfc, 0xf9, 0xf3, 0xe7, 0xcf, 0x9f, 0xff,
	    0xc3, 0x99, 0x91, 0x89, 0x99, 0x99, 0xc3, 0xff,
	    0xe7, 0xe7, 0xc7, 0xe7, 0xe7, 0xe7, 0x81, 0xff,
	    0xc3, 0x99, 0xf9, 0xf3, 0xcf, 0x9f, 0x81, 0xff,
	    0xc3, 0x99, 0xf9, 0xe3, 0xf9, 0x99, 0xc3, 0xff,
	    0xf9, 0xf1, 0xe1, 0x99, 0x80, 0xf9, 0xf9, 0xff,
	    0x81, 0x9f, 0x83, 0xf9, 0xf9, 0x99, 0xc3, 0xff,
	    0xc3, 0x99, 0x9f, 0x83, 0x99, 0x99, 0xc3, 0xff,
	    0x81, 0x99, 0xf3, 0xe7, 0xe7, 0xe7, 0xe7, 0xff,
	    0xc3, 0x99, 0x99, 0xc3, 0x99, 0x99, 0xc3, 0xff,
	    0xc3, 0x99, 0x99, 0xc1, 0xf9, 0x99, 0xc3, 0xff,
	    0xff, 0xff, 0xe7, 0xff, 0xff, 0xe7, 0xff, 0xff,
	    0xff, 0xff, 0xe7, 0xff, 0xff, 0xe7, 0xe7, 0xcf,
	    0xf1, 0xe7, 0xcf, 0x9f, 0xcf, 0xe7, 0xf1, 0xff,
	    0xff, 0xff, 0x81, 0xff, 0x81, 0xff, 0xff, 0xff,
	    0x8f, 0xe7, 0xf3, 0xf9, 0xf3, 0xe7, 0x8f, 0xff,
	    0xc3, 0x99, 0xf9, 0xf3, 0xe7, 0xff, 0xe7, 0xff,
	    0xc3, 0x99, 0x91, 0x91, 0x9f, 0x9d, 0xc3, 0xff,
	    0xe7, 0xc3, 0x99, 0x81, 0x99, 0x99, 0x99, 0xff,
	    0x83, 0x99, 0x99, 0x83, 0x99, 0x99, 0x83, 0xff,
	    0xc3, 0x99, 0x9f, 0x9f, 0x9f, 0x99, 0xc3, 0xff,
	    0x87, 0x93, 0x99, 0x99, 0x99, 0x93, 0x87, 0xff,
	    0x81, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x81, 0xff,
	    0x81, 0x9f, 0x9f, 0x87, 0x9f, 0x9f, 0x9f, 0xff,
	    0xc3, 0x99, 0x9f, 0x91, 0x99, 0x99, 0xc3, 0xff,
	    0x99, 0x99, 0x99, 0x81, 0x99, 0x99, 0x99, 0xff,
	    0xc3, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xc3, 0xff,
	    0xe1, 0xf3, 0xf3, 0xf3, 0xf3, 0x93, 0xc7, 0xff,
	    0x99, 0x93, 0x87, 0x8f, 0x87, 0x93, 0x99, 0xff,
	    0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x9f, 0x81, 0xff,
	    0x9c, 0x88, 0x80, 0x94, 0x9c, 0x9c, 0x9c, 0xff,
	    0x99, 0x89, 0x81, 0x81, 0x91, 0x99, 0x99, 0xff,
	    0xc3, 0x99, 0x99, 0x99, 0x99, 0x99, 0xc3, 0xff,
	    0x83, 0x99, 0x99, 0x83, 0x9f, 0x9f, 0x9f, 0xff,
	    0xc3, 0x99, 0x99, 0x99, 0x99, 0xc3, 0xf1, 0xff,
	    0x83, 0x99, 0x99, 0x83, 0x87, 0x93, 0x99, 0xff,
	    0xc3, 0x99, 0x9f, 0xc3, 0xf9, 0x99, 0xc3, 0xff,
	    0x81, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xff,
	    0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0xc3, 0xff,
	    0x99, 0x99, 0x99, 0x99, 0x99, 0xc3, 0xe7, 0xff,
	    0x9c, 0x9c, 0x9c, 0x94, 0x80, 0x88, 0x9c, 0xff,
	    0x99, 0x99, 0xc3, 0xe7, 0xc3, 0x99, 0x99, 0xff,
	    0x99, 0x99, 0x99, 0xc3, 0xe7, 0xe7, 0xe7, 0xff,
	    0x81, 0xf9, 0xf3, 0xe7, 0xcf, 0x9f, 0x81, 0xff,
	    0xc3, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xc3, 0xff,
	    0xff, 0x9f, 0xcf, 0xe7, 0xf3, 0xf9, 0xfc, 0xff,
	    0xc3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xc3, 0xff,
	    0xe7, 0xc3, 0x99, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x81, 0xff,
	    0x9f, 0xcf, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff,
	    0xff, 0xff, 0xc3, 0xf9, 0xc1, 0x99, 0xc1, 0xff,
	    0xff, 0x9f, 0x9f, 0x83, 0x99, 0x99, 0x83, 0xff,
	    0xff, 0xff, 0xc3, 0x9f, 0x9f, 0x9f, 0xc3, 0xff,
	    0xff, 0xf9, 0xf9, 0xc1, 0x99, 0x99, 0xc1, 0xff,
	    0xff, 0xff, 0xc3, 0x99, 0x81, 0x9f, 0xc3, 0xff,
	    0xff, 0xf1, 0xe7, 0xc1, 0xe7, 0xe7, 0xe7, 0xff,
	    0xff, 0xff, 0xc1, 0x99, 0x99, 0xc1, 0xf9, 0x83,
	    0xff, 0x9f, 0x9f, 0x83, 0x99, 0x99, 0x99, 0xff,
	    0xff, 0xe7, 0xff, 0xc7, 0xe7, 0xe7, 0xc3, 0xff,
	    0xff, 0xf9, 0xff, 0xf9, 0xf9, 0xf9, 0xf9, 0xc3,
	    0xff, 0x9f, 0x9f, 0x93, 0x87, 0x93, 0x99, 0xff,
	    0xff, 0xc7, 0xe7, 0xe7, 0xe7, 0xe7, 0xc3, 0xff,
	    0xff, 0xff, 0x99, 0x80, 0x80, 0x94, 0x9c, 0xff,
	    0xff, 0xff, 0x83, 0x99, 0x99, 0x99, 0x99, 0xff,
	    0xff, 0xff, 0xc3, 0x99, 0x99, 0x99, 0xc3, 0xff,
	    0xff, 0xff, 0x83, 0x99, 0x99, 0x83, 0x9f, 0x9f,
	    0xff, 0xff, 0xc1, 0x99, 0x99, 0xc1, 0xf9, 0xf9,
	    0xff, 0xff, 0x83, 0x99, 0x9f, 0x9f, 0x9f, 0xff,
	    0xff, 0xff, 0xc1, 0x9f, 0xc3, 0xf9, 0x83, 0xff,
	    0xff, 0xe7, 0x81, 0xe7, 0xe7, 0xe7, 0xf1, 0xff,
	    0xff, 0xff, 0x99, 0x99, 0x99, 0x99, 0xc1, 0xff,
	    0xff, 0xff, 0x99, 0x99, 0x99, 0xc3, 0xe7, 0xff,
	    0xff, 0xff, 0x9c, 0x94, 0x80, 0xc1, 0xc9, 0xff,
	    0xff, 0xff, 0x99, 0xc3, 0xe7, 0xc3, 0x99, 0xff,
	    0xff, 0xff, 0x99, 0x99, 0x99, 0xc1, 0xf3, 0x87,
	    0xff, 0xff, 0x81, 0xf3, 0xe7, 0xcf, 0x81, 0xff,
	    0xe3, 0xcf, 0xcf, 0x1f, 0xcf, 0xcf, 0xe3, 0xff,
	    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xff,
	    0xc7, 0xf3, 0xf3, 0xf8, 0xf3, 0xf3, 0xc7, 0xff,
	    0xff, 0xff, 0x8f, 0x24, 0xf1, 0xff, 0xff, 0xff,
	    0xff, 0xf7, 0xe3, 0xc9, 0x9c, 0x9c, 0x80, 0xff
	};
	uint32_t mask{0x7ff};

};

#endif
