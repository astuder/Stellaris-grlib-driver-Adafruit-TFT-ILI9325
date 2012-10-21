//*****************************************************************************
//
// touch.h - Prototypes for the touch screen driver based on example provided
// with Texas Instruments StellarisWare
//
//*****************************************************************************

#ifndef __TOUCH_H__
#define __TOUCH_H__

//*****************************************************************************
//
// The lowest ADC reading assumed to represent a press on the screen.  Readings
// below this indicate no press is taking place.
//
//*****************************************************************************
#define TOUCH_MIN 150

//*****************************************************************************
//
// Prototypes for the functions exported by the touch screen driver.
//
//*****************************************************************************
extern volatile short g_sTouchX;
extern volatile short g_sTouchY;
extern short g_sTouchMin;
extern void TouchScreenIntHandler(void);
extern void TouchScreenInit(void);
extern void TouchScreenCallbackSet(long (*pfnCallback)(unsigned long ulMessage,
                                                       long lX, long lY));
extern long* TouchScreenCalibrate(void);
extern void TouchScreenCalibrationPoint(unsigned short sPointX, unsigned short sPointY, unsigned long ulPointIndex);

#endif // __TOUCH_H__
