Stellaris grlib driver for Adafruit TFT ILI9235/9238
====================================================

TI Stellaris Graphics Library driver for Adafruit's 320x240x18 Touch TFT display.

* http://adafruit.com/products/335
* 320 x 240 pixels, 18 bit colors
* ILI9325 or ILI9328 display controller
* Resistive touch screen

The graphic driver code was rewritten from scratch with some inspiration from
Adafruit's driver for Arduino. In particular the initialization sequence was
copied verbatim.

The touch driver code is from the StellarisWare example and extended with 
calibration functionality.

Wiring of pins is mostly following the Kentec Booster Pack:
* CS   A.7   chip select
* C/D  A.6   command/data mode
* WR   A.5   write strobe
* RD   A.4   read strobe
* RST  F.4   reset (not available on Kentec)
* BKLT F.3   backlight (not available on Kentec)
* DATA B     data line 0-7 on pins B.0-B.7
* XP   E.4   touch X+
* YP   E.5   touch Y+
* XN   A.3   touch X-
* YN   A.2   touch Y-

To use with TI Stellaris Launchpad Workshop lab 10:
* Copy source files into drivers directory of your grlib_demo workspace
* In grlib_demo.c replace Kentec320x240x16_ssd2119_8bit with
Adafruit320x240x16TouchTFT_ILI9325
* In CCS remove links to Kentec and Touch from /driver directory (right click
delete)
* In CCS remove include path that points to boards\ek-lm4f120xl
* Replace main() in grlib_demo.c with the code in main-fragment.c

Limitations:
* This driver only uses 16bit color instead of the full 18bit available
* PixelDrawMultiple tested only with StellarisWare example (i.e. only for 4bit)
* In my test setup touch only worked reliable after adding 0.1uF capacitors from
Y- and X- to GND
