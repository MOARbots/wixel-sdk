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
#include <reporting.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

#define NUM_WAYPOINTS		5

#define ROBOTID			0
#define TARGETID		2

#define REPORTSIZE		1024

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;

//Store the timestamp of the last received packet
uint32 lastpacketRobot, lastpacketTag, diff, lastprint;

QTuple TagRobot, TagGoal;
QTuple tags[NUM_WAYPOINTS+1];

// Array of wether each item in the 'tags' array is initialized
uint8 initTag[NUM_WAYPOINTS+1];

//print flags
BIT pf_olddata, pf_turnL, pf_turnR, pf_success, pf_fwd;


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

int knownTag(uint8 tag) {
  uint8 cur;
  int i;
  printf("KNOWNTAG!\n\r");
  for (i = 0; i < (NUM_WAYPOINTS+1); i++) {
    cur = readID(&tags[i]);

    if (cur == tag) {
      return i;
    }
    if (initTag[i] == (uint8) 255) {
      initTag[i] = 123;
      return i;
    }
  }
  return 255;
}

int counter;

void robotRadioService() {
  //packet handling
  if (!readstate) { //the idle state
    if (radioComRxAvailable()) {
      if ( checkHeader( radioComRxReceiveByte() ) ) {
        readstate=1;
      }
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

      if (readID(&packet) == ROBOTID) { //Robot packet found
        lastpacketRobot = getMs();
        for (iter=0; iter<5; iter++) {
          TagRobot.bytes[iter] = packet.bytes[iter];
        }
      }

      // if (readID(&packet) != ROBOTID) { //Target packet found
      else {
        int tagIndex;
        lastpacketTag = getMs();
        tagIndex = knownTag(readID(&packet));
        for (iter=0; iter<5; iter++) {
          tags[0].bytes[iter] = packet.bytes[iter];
          // TagGoal.bytes[iter] = packet.bytes[iter];
        }
      }
    }
  }

  //BIT pf_olddata, pf_turnL, pf_turnR, pf_success, pf_fwd;

  //robot movement handling
  if ( ( (getMs() - lastpacketTag) > 250 ) | ( (getMs() - lastpacketRobot) > 250 ) ) { //if the packet is too old
    Brake(); //stop
    pf_turnL = pf_turnR = pf_success = pf_fwd = 0;
    if (pf_olddata == 0) { //only print when we enter this state
      if (radioComTxAvailable()) {
        radioComTxSendByte('?'); pf_olddata = 1;
      }
    }
  }

  else { //data is not too old to operate on
    float goalAngle;
    float diffAngle;
    BIT turndir;
    goalAngle = calculateAngle(readX(&TagRobot), readY(&TagRobot), readX(&TagGoal), readY(&TagGoal)) ;
    diffAngle = computeTurn(readR(&TagRobot),goalAngle);
    turndir = turnDirection(readR(&TagRobot), goalAngle);
    pf_olddata = 0; //reset the print flag for old data, so that next time we find old data we report it again

    if ( fabsf(diffAngle) < 45 ) { //Within margins on angle

      pf_turnL = pf_turnR = pf_olddata = 0;
      if ( distance(readX(&TagRobot), readY(&TagRobot), readX(&TagGoal), readY(&TagGoal)) < (float)50 ) { //Within margins on distance
        Brake();
        pf_fwd = 0;
        if (pf_success == 0) {
          if (radioComTxAvailable()) {
            radioComTxSendByte('+'); pf_success = 1;
          }
        }
      }
      else { //Not within margins on distance -- move forward
        pf_success = 0;
        Forward();
        if (pf_fwd == 0) {
          if (radioComTxAvailable()) {
            radioComTxSendByte('M'); pf_fwd = 1;
          }

        }
      }

    }
    else { //not within margins

      pf_olddata = pf_success = pf_fwd = 0;
      if (turndir == 0) { //turn left
        pf_turnR = 0;
        Left();
        if (pf_turnL == 0) {
          if (radioComTxAvailable()) {
            radioComTxSendByte('L'); pf_turnL = 1;
          }
        }
      }
      else {
        pf_turnL = 0;
        Right();
        if (pf_turnR == 0) {
          if (radioComTxAvailable()) {
            radioComTxSendByte('R'); pf_turnR = 1;
          }
        }
      }

    }
  }

  if (counter == 20000) {
    int i;
    for (i = 0; i < (NUM_WAYPOINTS + 1); i++) {
      printf("id: %u, init: %u\n\r", readID(&tags[i]), initTag[i]);
    }
    printf("---------------------------\n\r");
    counter = 0;
  }

  counter++;

  sendReportRadio();
}

void main()
{
  int i;

    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0; //must be zero if we don't call radioComRxControlSignals() regularly.
    radioComInit();
    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    motorsInit();
    setLeftPWM(80);
    setRightPWM(80);

    lastpacketTag = lastpacketRobot=0;
    pf_olddata = pf_turnL = pf_turnR = pf_success = pf_fwd = 0;

    for (i = 0; i < (NUM_WAYPOINTS + 1); i++) {
      initTag[i] = (uint8) 255;
    }

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
