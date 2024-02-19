[contents](../README.md) | [previous chapter](memory.md) |

# Blitter

*...work in progress...*

## Introduction

Blitter is a virtual device that copies data (pixels) very fast from one memory area to another.

## Surface descriptors

Transparency is handled by color code ```0``` by default.

### 1-bit

### 2-bit

### 4-bit

*...will this be implemented?...*

### 8-bit

In 8-bit mode transparency is handled by a zero bit value as well. It is however possible to override this by turning on ```bit 0``` (the background bit) in ```flags_0```. Now, value ```0x00``` will be opaque with a color defined at place ```[0]``` in the color index table.

## Blitting

When blitting, everything is treated like ending up on an 8-bit surface, *e.g. ...*
