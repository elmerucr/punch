# punch

![icon](./docs/punch_icon_80x80.png)

Punch is a virtual computer system that draws inspiration from iconic computing platforms such as the Commodore 64, Amiga 500, and Atari ST. Notable features include:

* Utilization of the MC6809 CPU using the [MC6809](https://github.com/elmerucr/MC6809) library
* A total of 16MB of video RAM, 64KB directly accessible by the CPU
* Screen resolution of 320x180 pixels, refresh rate of 60Hz
* Blitter chip for fast graphics processing

## Table of Contents

* [Memory Model](docs/memory.md)
* [Blitter](docs/blitter.md)

## Screenshots

### Startup screen

![punch](./docs/20240317_screenshot_startup.png)

### Debug screen

![punch](./docs/20240317_screenshot_debug.png)

## Memory Map

### Addressable by MC6809 and Blitter

* ```$000-$0ff``` direct page (default after reset)
* ```$100-$3ff``` available ram and system stack pointer (768 bytes)
* ```$0400-$0fff``` input/output area
	* ```$400-$4ff``` blitter surface descriptors (16 in total, 16 bytes each)
		* ```$x0/$x1```: x position (16 bit signed)
		* ```$x2/$x3```: y position (16 bit signed)
	* ```$500-$5ff``` blitter surface color tables (16) for 1, 2 and 4 bit color modes
	* ```$600-$7ff``` reserved area
	* ```$800-$8ff``` core
		* ```$800``` status register
		* ```$801``` control register
		* ```$802-$803``` vram peek page (16bits, bit 8 to bit 23)
		* ```$804-$807``` framebuffer base address (24 bits, $804 always #$00)
	* ```$900-$9ff``` keyboard
	* ```$a00-$aff``` timer
	* ```$b00-$bff``` reserved area
	* ```$c00-$dff``` sound
	* ```$e00-$eff``` blitter
		* ```$e00``` status register (unused)
		* ```$e01``` control register
			* write ```0b00000001```: blit source to destination surface
			* write ```0b00000010```: tile blit source/dest/tile
			* write ```0b00000100```: clear destination surface with drawing color
		* ```$e02``` source surface pointer (lowest nibble only)
		* ```$e03``` destination surface pointer (lowest nibble only)
		* ```$e04``` tile surface pointer (lowest nibble)
		* ```$e05``` drawing color
		* ```$e08/$e09``` x0 for drawing operations (16 bit signed)
		* ```$e0a/$e0b``` y0 for drawing operations (16 bit signed)
		* ```$e0c/$e0d``` x1 for drawing operations (16 bit signed)
		* ```$e0e/$e0f``` y1 for drawing operations (16 bit signed)
	* ```$f00-$fff``` vram peek (see $802/$803 as well)
* ```$1000-$fbff``` 59kb ram
* ```$fc00-$ffff``` 1kb kernel + vectors

### Directly addressable by Blitter

* ```$010000-$feffff``` available vram (16.256kb)
* ```$ff0000-$ffe0ff``` default framebuffer vram (57.600 bytes)
* ```$ffe100-$ffffff``` available vram (7.936 bytes)
