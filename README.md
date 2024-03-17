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

### Addressable by Blitter & MC6809

* ```$000000-$0000ff``` direct page (after reset)
* ```$000100-$0003ff``` available ram and system stack pointer (768 bytes)
* ```$000400-$000fff``` io
* ```$001000-$00fdff``` 60kb ram (minus 512 bytes)
* ```$00fe00-$00ffff``` 512 bytes kernel + vectors

### Addressable by Blitter only

* ```$010000-$0dffff``` available vram (832kb)
* ```$0e0000-$0fffff``` framebuffer vram (128kb)
* ```$100000-$ffffff``` framebuffer vram (15mb)
