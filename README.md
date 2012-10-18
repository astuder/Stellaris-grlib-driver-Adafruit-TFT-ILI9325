Stellaris grlib driver for Adafruit TFT ILI9235/9238
====================================================

TI Stellaris Graphics Library driver for Adafruit's 320x240x18 Touch TFT display.

Display
* http://adafruit.com/products/335
* 320 x 240 pixel, 18 bit colors
* ILI9325 or ILI9328 display controller

This code was rewritten from scratch with some inspiration from Adafruit's driver
for Arduino. In particular the initialization sequence was copied verbatim.

Wiring of pins is mostly following the Kentec Booster Pack:
* CS   A.7   chip select
* C/D  A.6   command/data mode
* WR   A.5   write strobe
* RD   A.4   read strobe
* RST  F.4   reset (not available on Kentec)
* BKLT F.3   backlight (not available on Kentec)
* DATA B     data line 0-7 on pins B.0-B.7

To use with TI Stellaris Launchpad Workshop lab 10:
* Copy source files into driver directory of grlib_demo workspace
* In grlib_demo.c replace Kentec320x240x16_ssd2119_8bit with
Adafruit320x240x16TouchTFT_ILI9325

Limitations:
* This driver only uses 16bit color instead of the available 18bit 
* PixelDrawMultiple tested only with StellarisWare example (i.e. only for 4bit)
* No support for touch functionality yet. This display requires different
calibration data than the Kentec booster pack used as example by TI.
