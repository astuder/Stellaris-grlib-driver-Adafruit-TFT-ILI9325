//
// Adafruit320x240x16TouchTFT_ILI9325.c
//
// Stellaris Graphics Library driver for Adafruit 320x240
// 16 bit color touch TFT breakout board w/ ILI9325 controller
// http://adafruit.com/products/335
//
// Copyright (c) 2012, Adrian Studer
// All rights reserved.
//
// License: (MIT License)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "grlib/grlib.h"
#include "Adafruit320x240x16TouchTFT_ILI9325.h"

// Physical dimensions of LCD
#define LCD_X	240
#define LCD_Y	320

//#define PORTRAIT or LANDSCAPE
#define LANDSCAPE

// Logical dimensions of LCD
#ifdef PORTRAIT
#define LCD_WIDTH	LCD_X
#define LCD_HEIGHT	LCD_Y
#else
#define LCD_WIDTH	LCD_Y
#define LCD_HEIGHT	LCD_X
#endif

// Port and bitmask used for 8-bit data bus
#define LCD_DATA_PERIPH			SYSCTL_PERIPH_GPIOB
#define LCD_DATA_BASE			GPIO_PORTB_BASE
#define LCD_DATA_PINS			0xFF

// Ports and pins used for control
#define LCD_CS_PERIPH			SYSCTL_PERIPH_GPIOA
#define LCD_CS_BASE				GPIO_PORTA_BASE
#define LCD_CS_PIN				GPIO_PIN_7
#define LCD_CD_PERIPH			SYSCTL_PERIPH_GPIOA
#define LCD_CD_BASE				GPIO_PORTA_BASE
#define LCD_CD_PIN				GPIO_PIN_6
#define LCD_WR_PERIPH			SYSCTL_PERIPH_GPIOA
#define LCD_WR_BASE				GPIO_PORTA_BASE
#define LCD_WR_PIN				GPIO_PIN_5
#define LCD_RD_PERIPH			SYSCTL_PERIPH_GPIOA
#define LCD_RD_BASE				GPIO_PORTA_BASE
#define LCD_RD_PIN				GPIO_PIN_4
#define LCD_RST_PERIPH			SYSCTL_PERIPH_GPIOF
#define LCD_RST_BASE			GPIO_PORTF_BASE
#define LCD_RST_PIN				GPIO_PIN_4
#define LCD_BKLT_PERIPH			SYSCTL_PERIPH_GPIOF
#define LCD_BKLT_BASE			GPIO_PORTF_BASE
#define LCD_BKLT_PIN			GPIO_PIN_3

// Macro to translate 24 bit RGB to 5-6-5 16 bit RGB
#define COLOR24TO16BIT(rgb)		((((rgb) & 0x00f80000) >> 8) | (((rgb) & 0x0000fc00) >> 5) | (((rgb) & 0x000000f8) >> 3))

// Macros for common pin operation, using direct memory access for performance and compactness
#define LCD_CS_IDLE 			HWREG(LCD_CS_BASE + GPIO_O_DATA + (LCD_CS_PIN << 2)) = LCD_CS_PIN;
#define LCD_CS_ACTIVE			HWREG(LCD_CS_BASE + GPIO_O_DATA + (LCD_CS_PIN << 2)) = 0;
#define LCD_CD_DATA 			HWREG(LCD_CD_BASE + GPIO_O_DATA + (LCD_CD_PIN << 2)) = LCD_CD_PIN;
#define LCD_CD_COMMAND			HWREG(LCD_CD_BASE + GPIO_O_DATA + (LCD_CD_PIN << 2)) = 0;
#define LCD_WR_IDLE	 			HWREG(LCD_WR_BASE + GPIO_O_DATA + (LCD_WR_PIN << 2)) = LCD_WR_PIN;
#define LCD_WR_ACTIVE			HWREG(LCD_WR_BASE + GPIO_O_DATA + (LCD_WR_PIN << 2)) = 0;
#define LCD_RD_IDLE	 			HWREG(LCD_RD_BASE + GPIO_O_DATA + (LCD_RD_PIN << 2)) = LCD_RD_PIN;
#define LCD_RD_ACTIVE			HWREG(LCD_RD_BASE + GPIO_O_DATA + (LCD_RD_PIN << 2)) = 0;
#define LCD_RST_IDLE			HWREG(LCD_RST_BASE + GPIO_O_DATA + (LCD_RST_PIN << 2)) = LCD_RST_PIN;
#define LCD_RST_ACTIVE			HWREG(LCD_RST_BASE + GPIO_O_DATA + (LCD_RST_PIN << 2)) = 0;
#define LCD_BKLT_ON				HWREG(LCD_BKLT_BASE + GPIO_O_DATA + (LCD_BKLT_PIN << 2)) = LCD_BKLT_PIN;
#define LCD_BKLT_OFF			HWREG(LCD_BKLT_BASE + GPIO_O_DATA + (LCD_BKLT_PIN << 2)) = 0;
#define LCD_DATA_WRITE(ucByte)	{ HWREG(LCD_DATA_BASE + GPIO_O_DATA + (LCD_DATA_PINS << 2)) = (ucByte); }

// Macro to wait x ms
#define LCD_DELAY(x)			{ SysCtlDelay((x) * (g_ulWait1ms)); }

// display controller register names from Peter Barrett's / Adafruit's Microtouch code
#define ILI_START_OSC 0x00
#define ILI_DRIV_OUT_CTRL 0x01
#define ILI_DRIV_WAV_CTRL 0x02
#define ILI_ENTRY_MOD 0x03
#define ILI_RESIZE_CTRL 0x04
#define ILI_DISP_CTRL1 0x07
#define ILI_DISP_CTRL2 0x08
#define ILI_DISP_CTRL3 0x09
#define ILI_DISP_CTRL4 0x0A
#define ILI_RGB_DISP_IF_CTRL1 0x0C
#define ILI_FRM_MARKER_POS 0x0D
#define ILI_RGB_DISP_IF_CTRL2 0x0F
#define ILI_POW_CTRL1 0x10
#define ILI_POW_CTRL2 0x11
#define ILI_POW_CTRL3 0x12
#define ILI_POW_CTRL4 0x13
#define ILI_GRAM_HOR_AD 0x20
#define ILI_GRAM_VER_AD 0x21
#define ILI_RW_GRAM 0x22
#define ILI_POW_CTRL7 0x29
#define ILI_FRM_RATE_COL_CTRL 0x2B
#define ILI_GAMMA_CTRL1 0x30
#define ILI_GAMMA_CTRL2 0x31
#define ILI_GAMMA_CTRL3 0x32
#define ILI_GAMMA_CTRL4 0x35
#define ILI_GAMMA_CTRL5 0x36
#define ILI_GAMMA_CTRL6 0x37
#define ILI_GAMMA_CTRL7 0x38
#define ILI_GAMMA_CTRL8 0x39
#define ILI_GAMMA_CTRL9 0x3C
#define ILI_GAMMA_CTRL10 0x3D
#define ILI_HOR_START_AD 0x50
#define ILI_HOR_END_AD 0x51
#define ILI_VER_START_AD 0x52
#define ILI_VER_END_AD 0x53
#define ILI_GATE_SCAN_CTRL1 0x60
#define ILI_GATE_SCAN_CTRL2 0x61
#define ILI_GATE_SCAN_CTRL3 0x6A
#define ILI_PART_IMG1_DISP_POS 0x80
#define ILI_PART_IMG1_START_AD 0x81
#define ILI_PART_IMG1_END_AD 0x82
#define ILI_PART_IMG2_DISP_POS 0x83
#define ILI_PART_IMG2_START_AD 0x84
#define ILI_PART_IMG2_END_AD 0x85
#define ILI_PANEL_IF_CTRL1 0x90
#define ILI_PANEL_IF_CTRL2 0x92
#define ILI_PANEL_IF_CTRL3 0x93
#define ILI_PANEL_IF_CTRL4 0x95
#define ILI_PANEL_IF_CTRL5 0x97
#define ILI_PANEL_IF_CTRL6 0x98

// Fake commands for display initialization script
#define ILI_DELAYCMD 0xFF
#define ILI_STOPCMD 0xFE

// Script to initialize display, copied from Adafruit library
static const unsigned short usInitScript[] =
{
	ILI_START_OSC, 0x0001, // start oscillator
	ILI_DELAYCMD, 50, // this will make a delay of 50 milliseconds
	ILI_DRIV_OUT_CTRL, 0x0100,
	ILI_DRIV_WAV_CTRL, 0x0700,
	ILI_ENTRY_MOD, 0x1030,
	ILI_RESIZE_CTRL, 0x0000,
	ILI_DISP_CTRL2, 0x0202,
	ILI_DISP_CTRL3, 0x0000,
	ILI_DISP_CTRL4, 0x0000,
	ILI_RGB_DISP_IF_CTRL1, 0x0,
	ILI_FRM_MARKER_POS, 0x0,
	ILI_RGB_DISP_IF_CTRL2, 0x0,
	ILI_POW_CTRL1, 0x0000,
	ILI_POW_CTRL2, 0x0007,
	ILI_POW_CTRL3, 0x0000,
	ILI_POW_CTRL4, 0x0000,
	ILI_DELAYCMD, 200,
	ILI_POW_CTRL1, 0x1690,
	ILI_POW_CTRL2, 0x0227,
	ILI_DELAYCMD, 50,
	ILI_POW_CTRL3, 0x001A,
	ILI_DELAYCMD, 50,
	ILI_POW_CTRL4, 0x1800,
	ILI_POW_CTRL7, 0x002A,
	ILI_DELAYCMD,50,
	ILI_GAMMA_CTRL1, 0x0000,
	ILI_GAMMA_CTRL2, 0x0000,
	ILI_GAMMA_CTRL3, 0x0000,
	ILI_GAMMA_CTRL4, 0x0206,
	ILI_GAMMA_CTRL5, 0x0808,
	ILI_GAMMA_CTRL6, 0x0007,
	ILI_GAMMA_CTRL7, 0x0201,
	ILI_GAMMA_CTRL8, 0x0000,
	ILI_GAMMA_CTRL9, 0x0000,
	ILI_GAMMA_CTRL10, 0x0000,
	ILI_GRAM_HOR_AD, 0x0000,
	ILI_GRAM_VER_AD, 0x0000,
	ILI_HOR_START_AD, 0x0000,
	ILI_HOR_END_AD, 0x00EF,
	ILI_VER_START_AD, 0X0000,
	ILI_VER_END_AD, 0x013F,
	ILI_GATE_SCAN_CTRL1, 0xA700, // Driver Output Control (R60h)
	ILI_GATE_SCAN_CTRL2, 0x0003, // Driver Output Control (R61h)
	ILI_GATE_SCAN_CTRL3, 0x0000, // Driver Output Control (R62h)
	ILI_PANEL_IF_CTRL1, 0X0010, // Panel Interface Control 1 (R90h)
	ILI_PANEL_IF_CTRL2, 0X0000,
	ILI_PANEL_IF_CTRL3, 0X0003,
	ILI_PANEL_IF_CTRL4, 0X1100,
	ILI_PANEL_IF_CTRL5, 0X0000,
	ILI_PANEL_IF_CTRL6, 0X0000,
	// Display On
	ILI_DISP_CTRL1, 0x0133, // Display Control (R07h)
	// Stop value / end of script
	ILI_STOPCMD
};

static const tRectangle g_FullScreen = { 0, 0, LCD_WIDTH-1, LCD_HEIGHT-1 };

// SysCtlDelay loops for 1 ms wait
unsigned long g_ulWait1ms;
unsigned short g_usPosX;
unsigned short g_usPosY;

void LCDWriteData(const unsigned short usData)
{
	LCD_CD_DATA

	// Send higher byte
	LCD_DATA_WRITE(usData >> 8);

	// Strobe WR
	LCD_WR_ACTIVE
	LCD_WR_IDLE

	// Send lower byte
	LCD_DATA_WRITE(usData & 0xff);

	// Strobe WR
	LCD_WR_ACTIVE
	LCD_WR_IDLE
}

void LCDWriteCommand(const unsigned short usAddress)
{
	LCD_CD_COMMAND

	// Send higher byte
	LCD_DATA_WRITE(usAddress >> 8);

	// Strobe WR
	LCD_WR_ACTIVE
	LCD_WR_IDLE

	// Send lower byte
	LCD_DATA_WRITE(usAddress & 0xff);

	// Strobe WR
	LCD_WR_ACTIVE
	LCD_WR_IDLE
}

// Coordinates of next display write
void LCDGoto(unsigned short x, unsigned short y)
{
	if(x >= LCD_WIDTH) x = LCD_WIDTH - 1;
	if(y >= LCD_HEIGHT) y = LCD_HEIGHT - 1;

#ifdef PORTRAIT
	LCDWriteCommand(ILI_GRAM_HOR_AD);	// GRAM Address Set (Horizontal Address) (R20h)
	LCDWriteData(x);
	LCDWriteCommand(ILI_GRAM_VER_AD);	// GRAM Address Set (Vertical Address) (R21h)
	LCDWriteData(y);
	LCDWriteCommand(ILI_RW_GRAM);		// Write Data to GRAM (R22h)
#else
	LCDWriteCommand(ILI_GRAM_HOR_AD);	// GRAM Address Set (Horizontal Address) (R20h)
	LCDWriteData(LCD_X - y);
	LCDWriteCommand(ILI_GRAM_VER_AD);	// GRAM Address Set (Vertical Address) (R21h)
	LCDWriteData(x);
	LCDWriteCommand(ILI_RW_GRAM);		// Write Data to GRAM (R22h)
#endif

	g_usPosX = x;
	g_usPosY = y;
}

// Clear display
void LCDClear(void)
{
	LCDGoto(0, 0);
	int i = 0;
	while(i < LCD_WIDTH * LCD_HEIGHT)
	{
		LCDWriteData(0);	// Write black pixel
		i++;
	}
}

// Configure display controller to write to defined display area
void LCDAddressWindow(const tRectangle *pRect)
{
#ifdef PORTRAIT
	LCDWriteCommand(ILI_HOR_START_AD);
	LCDWriteData(pRect->sXMin);
	LCDWriteCommand(ILI_HOR_END_AD);
	LCDWriteData(pRect->sXMax);
	LCDWriteCommand(ILI_VER_START_AD);
	LCDWriteData(pRect->sYMin);
	LCDWriteCommand(ILI_VER_END_AD);
	LCDWriteData(pRect->sYMax);
#else
	LCDWriteCommand(ILI_HOR_START_AD);
	LCDWriteData(LCD_X - pRect->sYMax);
	LCDWriteCommand(ILI_HOR_END_AD);
	LCDWriteData(LCD_X - pRect->sYMin);
	LCDWriteCommand(ILI_VER_START_AD);
	LCDWriteData(pRect->sXMin);
	LCDWriteCommand(ILI_VER_END_AD);
	LCDWriteData(pRect->sXMax);
#endif

	// Set pointer to first address in that window
	LCDGoto(pRect->sXMin, pRect->sYMin);
}

// Initializing display
void Adafruit320x240x16_ILI9325Init(void)
{
	unsigned short usAddress, usData;

	// Reset global variables
	g_ulWait1ms = SysCtlClockGet() / (3 * 1000);

	// Enable GPIO peripherals
	SysCtlPeripheralEnable(LCD_DATA_PERIPH);
    SysCtlPeripheralEnable(LCD_CS_PERIPH);
	SysCtlPeripheralEnable(LCD_CD_PERIPH);
    SysCtlPeripheralEnable(LCD_WR_PERIPH);
    SysCtlPeripheralEnable(LCD_RD_PERIPH);
    SysCtlPeripheralEnable(LCD_RST_PERIPH);
    SysCtlPeripheralEnable(LCD_BKLT_PERIPH);

    // Configure pins, all output
    GPIOPinTypeGPIOOutput(LCD_DATA_BASE, LCD_DATA_PINS);
    GPIOPinTypeGPIOOutput(LCD_CS_BASE, LCD_CS_PIN);
    GPIOPinTypeGPIOOutput(LCD_CD_BASE, LCD_CD_PIN);
    GPIOPinTypeGPIOOutput(LCD_WR_BASE, LCD_WR_PIN);
    GPIOPinTypeGPIOOutput(LCD_RD_BASE, LCD_RD_PIN);
    GPIOPinTypeGPIOOutput(LCD_BKLT_BASE, LCD_BKLT_PIN);
    GPIOPinTypeGPIOOutput(LCD_RST_BASE, LCD_RST_PIN);

    // Set control pins to idle/off state
    LCD_CS_IDLE
    LCD_RD_IDLE
    LCD_WR_IDLE
    LCD_BKLT_OFF

	// Reset LCD
    LCD_RST_ACTIVE
	LCD_DELAY(50);
	LCD_RST_IDLE
	LCD_DELAY(50);

	// Talk to LCD for init
	LCD_CS_ACTIVE

	// Sync communication
	LCDWriteData(0);
	LCDWriteData(0);
	LCDWriteData(0);
	LCDWriteData(0);
	LCD_DELAY(50);

	// Process initialization sequence of display driver
	int i = 0;
	while(usInitScript[i] != ILI_STOPCMD)
	{
		usAddress = usInitScript[i++];
		usData = usInitScript[i++];

		if(usAddress == ILI_DELAYCMD)
		{
			LCD_DELAY(usData);
		}
		else
		{
			LCDWriteCommand(usAddress);
			LCDWriteData(usData);
		}
	}

	// Clear display of any stray pixels
	LCDClear();

	// Done talking to LCD
	LCD_CS_IDLE

	// Turn back light on
	LCD_BKLT_ON

	return;
}

void Adafruit320x240x16_ILI9325PixelDraw(void *pvDisplayData, long lX, long lY, unsigned long ulValue)
{
	// Start talking to LCD
	LCD_CS_ACTIVE

	LCDGoto(lX, lY);
	LCDWriteData(ulValue);

	// Done talking to LCD
	LCD_CS_IDLE
}

void Adafruit320x240x16_ILI9325PixelDrawMultiple(void *pvDisplayData,
												 long lX, long lY, long lX0, long lCount, long lBPP,
												 const unsigned char *pucData,
												 const unsigned char *pucPalette)
{
	// Start talking to LCD
	LCD_CS_ACTIVE

#ifdef LANDSCAPE
	// Configure write direction to horizontal
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1038);
#endif

	LCDGoto(lX,lY);

	unsigned long ulPixel = 0;
	unsigned long ulColor = 0;

    if(lBPP == 1)
    {
    	// 1 bit per pixel in pucData
    	// lX0 is the index of the bit processed within a byte
    	// pucPalette holds the pre-translated 32bit display color
    	while(lCount)
    	{
    		ulPixel = *pucData++;

    		while(lCount && lX0 < 8)	// while there are pixels in this byte
    		{
    			ulColor = ((unsigned long *)pucPalette)[ulPixel & 1];	// retrieve already translated color
    			LCDWriteData(ulColor);

    			lCount--;		// processed another pixel
    			lX0++;			// done with this bit
    			ulPixel >>= 1;	// prepare next bit
    		}

    		lX0 = 0;	// process next byte, reset bit counter
    	}
    }
    else if(lBPP == 4)
    {
    	// 4 bits per pixel in pucData
    	// lX0 holds 0/1 to indicate 4-bit nibble within byte
    	// pucPalette holds untranslated 24 bit color
    	while(lCount)
    	{
    		if(lX0 == 0)	// read first nibble
    		{
    			ulPixel = *pucData >> 4;
    			lX0 = 1;	// set index to second nibble
    		}
    		else
    		{				// read second nibble
    			ulPixel = *pucData & 0x0f;
    			pucData++;	// increase byte pointer as we're done reading this byte
    			lX0 = 0;	// set index to first nibble
    		}

			ulColor = *(unsigned long *)(pucPalette + (ulPixel*3)) & 0x00ffffff;	// retrieve 24 bit color
			LCDWriteData(COLOR24TO16BIT(ulColor));					// translate and write to display

			lCount--;	// processed another pixel
    	}
    }
    else if(lBPP == 8)
    {
    	// 8 bits per pixel in pucData
    	// pucPalette holds untranslated 24 bit color
    	while(lCount)
    	{
   			ulPixel = *pucData++;		// read pixel
			ulColor = *(unsigned long *)(pucPalette + (ulPixel*3)) & 0x00ffffff;	// retrieve 24 bit color
			LCDWriteData(COLOR24TO16BIT(ulColor));		// translate color and write to display
			lCount--;	// processed another pixel
    	}
    }
    else if(lBPP == 16)
    {
    	// 16 bits per pixel
    	// Pixel is in 16bit color, 5R 6G 5B format
        // No color translation needed for this display
        while(lCount)
        {
        	ulPixel = *((unsigned short *)pucData);
            LCDWriteData(ulPixel);
            pucData += 2;
            lCount--;
        }
    }

#ifdef LANDSCAPE
	// Reset write direction to default (vertical)
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1030);
#endif

	// Done talking to LCD
	LCD_CS_IDLE
}

void Adafruit320x240x16_ILI9325LineDrawH(void *pvDisplayData,
										 long lX1, long lX2, long lY, unsigned long ulValue)
{
	// Start talking to LCD
	LCD_CS_ACTIVE

#ifdef LANDSCAPE
	// 	Configure write direction to horizontal
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1038);
#endif

	long x = lX1;
	LCDGoto(x, lY);
	while(x <= lX2)
	{
		LCDWriteData(ulValue);
		x++;
	}

#ifdef LANDSCAPE
	// Reset write direction to default (vertical)
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1030);
#endif

	// Done talking to LCD
	LCD_CS_IDLE
}

void Adafruit320x240x16_ILI9325LineDrawV(void *pvDisplayData,
										 long lX, long lY1, long lY2, unsigned long ulValue)
{
	// Start talking to LCD
	LCD_CS_ACTIVE

#ifdef PORTRAIT
	// Configure write direction to horizontal
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1018);
#endif

	long y = lY1;

	LCDGoto(lX, lY2);

	while(y <= lY2)
	{
		LCDWriteData(ulValue);
		y++;
	}

#ifdef PORTRAIT
	// Reset write direction to default (vertical)
	LCDWriteCommand(ILI_ENTRY_MOD);
	LCDWriteData(0x1030);
#endif

	// Done talking to LCD
	LCD_CS_IDLE
}

void Adafruit320x240x16_ILI9325RectFill(void *pvDisplayData,
										const tRectangle *pRect, unsigned long ulValue)
{
	// Start talking to LCD
	LCD_CS_ACTIVE

	LCDAddressWindow(pRect);

	int i = 0;
	int pixel = (pRect->i16XMax - pRect->i16XMin + 1) * (pRect->i16YMax - pRect->i16YMin + 1);

	while(i < pixel)
	{
		LCDWriteData(ulValue);	// Write pixel
		i++;
	}

	LCDAddressWindow(&g_FullScreen);

	// Done talking to LCD
	LCD_CS_IDLE
}

static unsigned long Adafruit320x240x16_ILI9325ColorTranslate(void *pvDisplayData, unsigned long ulValue)
{
    //
    // Translate from a 24-bit RGB color to a 5-6-5 RGB color.
    //
    return(COLOR24TO16BIT(ulValue));
}

static void Adafruit320x240x16_ILI9325Flush(void *pvDisplayData)
{
    // nothing to do as we write directly to display
}

// grlib structure describing Adafruit 320x240x16 TFT Touch Display driver.
const tDisplay g_sAdafruit320x240x16_ILI9325 =
{
		sizeof(tDisplay),								// size of this structure
		0,												// ptr to display specific data(?)
		LCD_WIDTH,										// width in pixel
		LCD_HEIGHT,										// height in pixel
		Adafruit320x240x16_ILI9325PixelDraw,			// function to draw pixel
		Adafruit320x240x16_ILI9325PixelDrawMultiple,	// function to draw multiple pixel
		Adafruit320x240x16_ILI9325LineDrawH,			// function to draw horizontal line
		Adafruit320x240x16_ILI9325LineDrawV,			// function to draw vertical line
		Adafruit320x240x16_ILI9325RectFill,				// function to fill rectangle
		Adafruit320x240x16_ILI9325ColorTranslate,		// function to translate 24bit color
		Adafruit320x240x16_ILI9325Flush					// function to flush display writes
};
