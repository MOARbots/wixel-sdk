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
#include <reporting.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;

/*Suggested use:
 * 0 means idle (no packet currently being read), 1 means a packet is
 * currently in the process of being collected off the radioRx buffer.
 * If the packet library checkheader(byte) function returns false,
 * check to see if you have an ASCII character instead (special robot instructions)
 * Consider that the header, 0xFC, means the byte is minimum 252.
 * The ASCII character range is only 0-127, so there should be no ambiguity.
 */
BIT readstate=0;

/** Functions *****************************************************************/


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

//This tests sendReportUSB()
//Input: StreamData formatted data (111111ii iiiyyyyy yyyyyxxx xxxxxxxr rrrrrrrr) received over the radio
//Output goes to the USB, which can be viewed with a serial terminal such as PuTTY, gtkterm, or screen
void usbToRadioService()
{
    if (!readstate) { //the idle state	
	if (radioComRxAvailable()) {
	    if ( checkHeader( radioComRxReceiveByte() ) ) { readstate=1; }
	}
    }

    else { //readstate
	if ( i + 1 < QTupleSize ) { //packet not yet full
	    if (radioComRxAvailable() > 0) { //if byte available
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
	    putchar('\n');
	    putchar('\r');
	}
    }
    //if there is something on the report buffer, send it
    sendReportUSB();
}

//This tests sendReportRadio()
//Input: StreamData formatted data (111111ii iiiyyyyy yyyyyxxx xxxxxxxr rrrrrrrr) received over the radio
//Output goes to the radio, which is received in turn by the other Wixel
void robotRadioService() {
    if (!readstate) { //the idle state	
	if (radioComRxAvailable()) {
	    if ( checkHeader( radioComRxReceiveByte() ) ) { readstate=1; }
	}
    }

    else { //readstate
	if ( i + 1 < QTupleSize ) { //packet not yet full
	    if (radioComRxAvailable() > 0) { //if byte available
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
	    putchar('\n');
	    putchar('\r');
	}
    }
    //if there is something on the report buffer, send it
    sendReportRadio();
}

void main()
{
    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0; //must be zero if we don't call radioComRxControlSignals() regularly.
    radioComInit();
    bufferInit();

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
