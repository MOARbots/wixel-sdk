/* Shira's waypoint navigator
 *
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
#include <motors.h>
#include <timer.h>
#include <trig_utils.h>
#include <math.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

#define NUM_WAYPOINTS		5

#define ROBOTID			26
#define TAGRADIUS		25

#define REPORTSIZE		1024

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;

//Store the timestamp of the last received packet
uint32 lastpacketRobot, lastpacketTag,diff,lastprint;

QTuple XDATA TagRobot;
QTuple XDATA TagGoal[NUM_WAYPOINTS];
uint8 tagIDs[5];
uint8 tagCount;

// A big buffer for holding a report.  This allows us to print more than
// 128 bytes at a time to USB.
uint8 XDATA report[REPORTSIZE];

// The length (in bytes) of the report currently in the report buffer.
// If zero, then there is no report in the buffer.
uint16 DATA reportLength = 0;

// The number of bytes of the current report that have already been
// send to the computer over USB.
uint16 DATA reportBytesSent = 0;

/*Suggested use:
 * 0 means idle (no packet currently being read), 1 means a packet is
 * currently in the process of being collected off the radioRx buffer.
 * If the packet library checkheader(byte) function returns false,
 * check to see if you have an ASCII character instead (special robot instructions)
 * Consider that the header, 0xFC, means the byte is minimum 252.
 * The ASCII character range is only 0-127, so there should be no ambiguity.
 */
BIT readstate=0;
BIT buffer_overflow = 0; //flag to check if a buffer overflow occured
BIT init_stage = 1;
BIT ready_signal = 0;

/** Functions *****************************************************************/

// This gets called by puts, printf. The result is sent by sendReport functions
void putchar(char c)
{
    if (reportLength >= REPORTSIZE) { buffer_overflow = 1; }
    else {
	report[reportLength] = c;
	reportLength++;
	buffer_overflow = 0;
    }
}

//This sends data over the radio
void sendReportRadio()
{
    uint8 bytesToSend, i;
    if (reportLength > 0) //Nonzero size report
    {
        bytesToSend = radioComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent) //Can send full report
        {
            // Iterate through byte by byte, sending the data until the end of the report.
            for (i=reportBytesSent; i < reportLength; i++  ) {
	    	radioComTxSendByte(report[i]);
	    }
            reportLength = 0; //reset the length
	    reportBytesSent = 0; //reset the progress counter
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
    if (reportLength > 0) //Nonzero size report
    {
        bytesToSend = usbComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent) //Can send full report
        {
            // Send the report from reportBytesSent until end of report
            usbComTxSend(report+reportBytesSent, reportLength - reportBytesSent);
            reportLength = 0; //reset the length
	    reportBytesSent = 0; //reset the progress counter
        }
        else //Not enough space on the buffers, send only what will fit
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
/*
    if (!readstate) { //the idle state
	if (radioComRxAvailable()) {
	    if ( checkHeader( radioComRxReceiveByte() ) ) { readstate=1; }
	}
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
*/
}

//keep in mind i is a counter used here and declared in packet.c
void robotRadioService() {
    sendReportRadio();
    //packet handling
    if (!readstate) { //the idle state
	if (radioComRxAvailable()) {
	    uint8 mybyte;
	    mybyte = radioComRxReceiveByte();
	    if ( checkHeader( mybyte ) ) { readstate=1; }
	    else if ( mybyte == 0x67 ) { ready_signal = 1; }
	}
    }

    else { //readstate
	if ( i + 1 < QTupleSize ) { //packet not yet full
	    if (radioComRxAvailable()) { //if byte available
	    	packet.bytes[i+1] = radioComRxReceiveByte(); //take it off the receiving buffer
		i = i+1;
	    }
	}
	else { //We filled up one packet
	    int iter;
	    readstate = 0; //return to idle state next loop
	    i=0;

	    if (init_stage == 1) { //initialization of tag locations
		if (readID(&packet) != ROBOTID) { //a tag packet was found
		    BIT exists=0;
		    for ( iter=0; iter < NUM_WAYPOINTS; iter++) {
			if (tagIDs[iter] == readID(&packet)) {exists=1;}
		    }
		    if (!exists) { //this tag was not yet located
			tagIDs[tagCount] = readID(&packet); //copy the ID to the tagIDs list
			for (iter=0; iter<5; iter++) { TagGoal[tagCount].bytes[iter] = packet.bytes[iter]; } //copy the tag to the TagGoal array
			tagCount++; //increment the tagCount
			if (tagCount > 4) { init_stage = 0; tagCount = 0; printf("Initialization complete. Seeking tag %u \n\r",tagIDs[0]); } //we found all tags, end init_stage
		    }
	    	}
	    }

	    else { //init stage is over
	    	if (readID(&packet) == ROBOTID) { //Robot packet found
		    lastpacketRobot = getMs();
		    for (iter=0; iter<5; iter++) { TagRobot.bytes[iter] = packet.bytes[iter]; }
	    	}
	    }
	}
    }

    //robot movement handling
    if ( init_stage == 1 | ( (getMs() - lastpacketRobot) > 500 | ready_signal == 0 ) | tagCount > (NUM_WAYPOINTS-1) ) { //if the packet is too old or we're not done with init stage or we've covered all points
	Brake(); //stop
    }
    else { //we should move
	float goalAngle;
	float diffAngle;
	BIT turndir;
  float dist = distance(readX(&TagRobot), readY(&TagRobot), readX(&TagGoal[tagCount]), readY(&TagGoal[tagCount]));
  float coneAngle;
	goalAngle = calculateAngle(readX(&TagRobot), readY(&TagRobot), readX(&TagGoal[tagCount]), readY(&TagGoal[tagCount])) ;
	diffAngle = computeTurn(readR(&TagRobot),goalAngle);
	turndir = turnDirection(readR(&TagRobot), goalAngle);

  coneAngle = (dist / (float) 10) + (float) 10;

	//if within margins on distance, success.
	if ( dist < (float) 19) { // Make sure it is counted
	    Brake();
	    printf("Reached tag %u. \n\r",tagIDs[tagCount]);
	    tagCount++;
	}

	else if ( fabsf(diffAngle) < coneAngle ) { //Within margins on angle
      if (dist > (float) 240) { // half of the board
        setLeftPWM(160);
        setRightPWM(160);
      } else if (dist > (float) 120) { // quarter of board
        setLeftPWM(145);
        setRightPWM(145);
      } else if (dist > (float) 45) { // eighth of board
        setLeftPWM(125);
        setRightPWM(125);
    } else { // less than that!
        setLeftPWM(100);
        setRightPWM(100);
      }
	    Forward();
	}
	  else { //not within margins
      setLeftPWM(60);
      setRightPWM(60);
	    if (turndir == 0) { //turn left
		    Left();
	    }
	    else {
    		Right();
	    }
    }
  }
}

void main()
{
    uint8 count;
    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0; //must be zero if we don't call radioComRxControlSignals() regularly.
    radioComInit();
    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    motorsInit();
    setLeftPWM(125);
    setRightPWM(125);
    setLeftOffset(10);
    setRightOffset(0);

    for (count=0; count < NUM_WAYPOINTS; count++){
	tagIDs[count] = 255; //initialize all these tags to 255, which is reserved to mean 'not yet set'
    }
    tagCount = 0;

    lastpacketTag = lastpacketRobot=0;

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
