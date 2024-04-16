# punch

![icon](./docs/punch_icon_80x80.png)

Punch is a virtual computer system that draws inspiration from iconic computing platforms such as the Commodore 64, Amiga 500, and Atari ST.
Notable features include:

* Utilization of the MC6809 CPU using the [MC6809](https://github.com/elmerucr/MC6809) library
* A total of 16MB of video RAM, 64KB directly accessible by the CPU
* Screen resolution of 320x180 pixels, refreshing rate of 60Hz
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
* ```$400-$4ff``` io - blitter surfaces
* ```$500-$5ff``` io - blitter color tables
* ```$600-$7ff``` io - reserved area
* ```$800-$8ff``` io - core
	* ```$800``` status register
	* ```$801``` control register
	* ```$802-$803``` vram peek page (16bits)
	* ```$804-$807``` framebuffer base address (24 bits, $804 always #$00)
* ```$900-$9ff``` io - keyboard
* ```$a00-$aff``` io - timer
* ```$b00-$bff``` io - reserved area
* ```$c00-$dff``` io - sound
* ```$e00-$eff``` io - blitter
* ```$f00-$fff``` io - vram peek (see $802 as well)
* ```$1000-$fbff``` 59kb ram
* ```$fc00-$ffff``` 1kb kernel + vectors

### Directly addressable by Blitter

* ```$010000-$feffff``` available vram (16.256kb)
* ```$ff0000-$ffe0ff``` default framebuffer vram (57.600 bytes)
* ```$ffe100-$ffffff``` available vram (73.472 bytes)
