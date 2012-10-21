// Code fragment to replace main() in grlib_demo.c

int
main(void)
{
    tContext sContext;
    tRectangle sRect;

    //
    // The FPU should be enabled because some compilers will use floating-
    // point registers, even for non-floating-point code.  If the FPU is not
    // enabled this will cause a fault.  This also ensures that floating-
    // point operations could be added to this application and would work
    // correctly and use the hardware floating-point unit.  Finally, lazy
    // stacking is enabled for interrupt handlers.  This allows floating-
    // point instructions to be used within interrupt handlers, but at the
    // expense of extra stack usage.
    //
    FPUEnable();
    FPULazyStackingEnable();

    //
    // Set the clock to 40Mhz derived from the PLL and the external oscillator
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    //
    // Initialize the display driver.
    //
    Adafruit320x240x16_ILI9325Init();

    //
    // Initialize the graphics context.
    //
    GrContextInit(&sContext, &g_sAdafruit320x240x16_ILI9325);

    //
    // Configure and enable uDMA
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    SysCtlDelay(10);
    uDMAControlBaseSet(&sDMAControlTable[0]);
    uDMAEnable();

    //
    // Initialize the touch screen driver and have it route its messages to the
    // widget tree.
    //
    TouchScreenInit();

    //
    // Paint touch calibration targets and collect calibration data
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrContextBackgroundSet(&sContext, ClrBlack);
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDraw(&sContext, "Touch center of circles to calibrate", -1, 0, 0, 1);
    GrCircleDraw(&sContext, 32, 24, 10);
    GrFlush(&sContext);
    TouchScreenCalibrationPoint(32, 24, 0);

    GrCircleDraw(&sContext, 280, 200, 10);
    GrFlush(&sContext);
    TouchScreenCalibrationPoint(280, 200, 1);

    GrCircleDraw(&sContext, 200, 40, 10);
    GrFlush(&sContext);
    TouchScreenCalibrationPoint(200, 40, 2);

    //
    // Calculate and set calibration matrix
    //
    long* plCalibrationMatrix = TouchScreenCalibrate();
    
    //
    // Write out calibration data if successful
    //
    if(plCalibrationMatrix)
    {
    	char pcStringBuf[20];
    	usprintf(pcStringBuf, "A %d", plCalibrationMatrix[0]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 20, 1);
    	usprintf(pcStringBuf, "B %d", plCalibrationMatrix[1]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 40, 1);
    	usprintf(pcStringBuf, "C %d", plCalibrationMatrix[2]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 60, 1);
    	usprintf(pcStringBuf, "D %d", plCalibrationMatrix[3]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 80, 1);
    	usprintf(pcStringBuf, "E %d", plCalibrationMatrix[4]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 100, 1);
    	usprintf(pcStringBuf, "F %d", plCalibrationMatrix[5]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 120, 1);
    	usprintf(pcStringBuf, "Div %d", plCalibrationMatrix[6]);
    	GrStringDraw(&sContext, pcStringBuf, -1, 0, 140, 1);
    	TouchScreenCalibrationPoint(0,0,0);	// wait for dummy touch
    }

    //
    // Enable touch screen event handler for grlib widgets
    //
    TouchScreenCallbackSet(WidgetPointerMessage);

    //
    // Fill the top 24 rows of the screen with blue to create the banner.
    //
    sRect.sXMin = 0;
    sRect.sYMin = 0;
    sRect.sXMax = GrContextDpyWidthGet(&sContext) - 1;
    sRect.sYMax = 23;
    GrContextForegroundSet(&sContext, ClrDarkBlue);
    GrRectFill(&sContext, &sRect);

    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&sContext, ClrWhite);
    GrRectDraw(&sContext, &sRect);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&sContext, &g_sFontCm20);
    GrStringDrawCentered(&sContext, "grlib demo", -1,
                         GrContextDpyWidthGet(&sContext) / 2, 8, 0);

    //
    // Add the title block and the previous and next buttons to the widget
    // tree.
    //
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sPrevious);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sTitle);
    WidgetAdd(WIDGET_ROOT, (tWidget *)&g_sNext);

    //
    // Add the first panel to the widget tree.
    //
    g_ulPanel = 0;
    WidgetAdd(WIDGET_ROOT, (tWidget *)g_psPanels);
    CanvasTextSet(&g_sTitle, g_pcPanelNames[0]);

    //
    // Issue the initial paint request to the widgets.
    //
    WidgetPaint(WIDGET_ROOT);

    //
    // Loop forever handling widget messages.
    //
    while(1)
    {
        //
        // Process any messages in the widget message queue.
        //
        WidgetMessageQueueProcess();
    }
}
