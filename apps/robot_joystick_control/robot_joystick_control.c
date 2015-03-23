#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <timer.h>
#include <motors.h>
#include <stdio.h>

#include "utils.h"
#include "updaters.h"
#include "services.h"

int32 CODE param_baud_rate = 9600;

uint8 DATA currentMode;

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to USB.
uint8 XDATA report[1024];

// The length (in bytes) of the report currently in the report buffer.
// If zero, then there is no report in the buffer.
uint16 DATA reportLength = 0;

// The number of bytes of the current report that have already been
// send to the computer over USB.
uint16 DATA reportBytesSent = 0;

// This gets called by puts, printf. The result is sent by sendReport()
void putchar(char c)
{
    report[reportLength] = c;
    reportLength++;
}

//This sends data over the radio
void sendReportRadio()
{
    uint8 bytesToSend, i;

    // Send the report to radio in chunks.
    if (reportLength > 0)
    {
        bytesToSend = radioComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent)
        {
            // Iterate through byte by byte, sending the data until the end of the report.
            for (i=reportBytesSent; i < reportLength; i++  ) {
	    	radioComTxSendByte(report[i]);
	    }
            reportLength = 0;
        }
        else //Not enough space on the buffers, send only what will fit
        {
	    for (i=reportBytesSent; i < (reportBytesSent+bytesToSend); i++) {
            	radioComTxSendByte(report[i]);
	    }
            reportBytesSent += bytesToSend;
        }
    }
}

//This sends data over the USB
void sendReportUSB()
{

    uint8 bytesToSend;

    // Send the report to USB in chunks.
    if (reportLength > 0)
    {
        bytesToSend = usbComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent)
        {
            // Send the last part of the report.
            usbComTxSend(report+reportBytesSent, reportLength - reportBytesSent);
            reportLength = 0;
        }
        else
        {
            usbComTxSend(report+reportBytesSent, bytesToSend);
            reportBytesSent += bytesToSend;
        }
    }
}

void main(void) {
	systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0;
    radioComInit();
    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    motorsInit();

    while (1) {
    	currentMode = updateMode();
    	boardService();
    	updateLEDs(); 
    	radioComTxService();
        usbComService();

        sendReportUSB();

    	switch (currentMode) {
	    	case MODE_TETHERED: computerLink(); break;
	    	case MODE_UNTETHERED: robot(); break;
	    }
    }
}