/* Wireless_serial_packet
 * Demonstrates packet.h library
 */

/** Dependencies **************************************************************/
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <packet.h>
#include <queue.h>
#include <string.h>
#include <stdio.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

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

/** Functions *****************************************************************/

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

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

    if (!radioLinkConnected())
    {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    }
    else
    {
        // We have connected.

        if ((now & 0x3FF) <= 20)
        {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        }
        else if (dimYellowLed)
        {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7)==0);
        }
        else
        {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred)
    {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        //dimYellowLed = 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32)
    {
        dimYellowLed = 0;
    }
}

void updateMode()
{
    if (usbPowerPresent())
    {
	currentMode = MODE_TETHERED;
    }
    else
    {
        currentMode = MODE_UNTETHERED;
    }
}

//This mode is active when the USB is plugged in
//Test Streaming Data to USB. Data is received over the radio.
//If the packet is properly formatted (111111 header, 5 bit ID, 10 bit Y pos, 10 bit X pos, 9 bit rotation, total 5 bytes)
//Then the Wixel will send this back via the PC it is connected to via USB.
void usbToRadioService()
{

    if (!readstate) { //the idle state	
	if (radioComRxAvailable()) { checkHeader( radioComRxReceiveByte() ); }
    }

    else { //readstate
	if ( i + 1 < QTupleSize ) { //packet not yet full
	    if (radioComRxAvailable()) { //if byte available
	    	packet.bytes[i+1] = radioComRxReceiveByte(); //take it off the receiving buffer
		i = i+1;
	    }
	}
	else { //We filled up one packet
	    readstate = 0; //return to idle state next loop
	    i=0;
	    
	    //Store or process the packet. Here we are formatting with printf, which calls putchar, then we call sendReport
	    printf("ID: %u, ", readID(&packet));
	    printf("Y: %u, ", readY(&packet));
	    printf("X: %u, ", readX(&packet));
	    printf("R: %u", readR(&packet));
	    putchar('\n'); //note output won't be left aligned properly in screen or gtkterm... TODO a fix for this?
	    putchar('\r'); //note output won't be left aligned properly in screen or gtkterm... TODO a fix for this?
	    sendReportUSB();
	}
    }
}

void robotRadioService() {
    //unpopulated, TODO add example that relays data back via sendReportRadio

}

void main()
{
    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0; //must be zero if we don't call radioComRxControlSignals() regularly.
    radioComInit();

    while(1)
    {
        updateMode();
        boardService();
        updateLeds();
        radioComTxService();
        usbComService();

        switch(currentMode)
        {
        case MODE_TETHERED:  	usbToRadioService();  break;
        case MODE_UNTETHERED: 	robotRadioService(); break;
        }
    }
}
