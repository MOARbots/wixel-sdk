/* wireless_serial_multichannel app
 * This app jumps from channel to channel and communicates over each as the wireless_serial does
 * It is appropriate only for camera server Wixel -> Robot Wixel communication because it doesn't not idle
 * long enough on any channel for the camera server Wixel to hear a response.
 * 
 * Modified by Shira based on the code modified by Geoff Nagy on July-20-2012.
 * IF we are in the default state, but the USB RX buffer has something to send, we
 * transition to the sending state, and make a temporary copy of the data to send.
 * In the sending state, we push the data to the TX buffer, wait for it to clear,
 * and then change the channel to the next in the list, and repeat. After every
 * channel has been dealt with, we return to default state.
 * A timeout on clearing the TX buffer means we move on eventually even if the wixel
 * on the other end is not responsive.
 * Every large interval of time, we check our list of decided upon channels by sending
 * out a symbol, and populate the channel list with those that got a response before
 * the timeout.
 * Whenever a Wixel is not responsive it joins a list of dead channels.
 * These dead channels are not attempted every round. Every HEARTBEAT_TIME,
 * the next dead channel in the list is attempted once. If successful, it returns
 * to the live list.
*/

/** Dependencies **************************************************************/
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <radio_registers.h>
#include <radio_mac.h>


/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

//Size of tempData array
#define MAX_TX_BYTES			18
// chosen arbitrarily; we just need a reasonable delay (does not overflow)
#define MAX_CHANNEL_CHANGE_TRIES 254
//Time between heartbeats in milliseconds
#define HEARTBEAT_TIME			500

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;

//Shira's global variables for MultiChannel
uint8 sendingState = 0; //0 is default, 1 is push data on current channel, 2 is attempt a channel change
uint8 channelCounter = 0; //Counter that indexes which channel we are to send on next
uint8 deadchannelCounter = 255; //If it starts live, shouldn't matter 
uint8 channelList[16] = {0,17,34,51,68,85,102,119,136,153,170,187,204,221,238,255}; //A list to store what channels are active. Updated when we check for alive wixels
uint16 channelStates = 0xFFFF; //A binary 1 for live, 0 for dead. MSB represents the 16th channel in the list; LSB represents the 0th channel in the list.
//Initially we assume all channels live. As the timeouts happen, they will be dropped.
uint8 lastChannelIndex = 15; //The last index of the channelList array we should look at.
uint8 numTempBytes = 0; //A place to store the current length < MAX_TX_BYTES of the tempData structure 
uint8 tempData [MAX_TX_BYTES]; //structure to store our data while we repeat it on all channels. See note below
BIT rollover = 0; //This is set to 1 by findNextChannel and allows sendingState 2 to proceed to default state 0 after a rollover
uint32 lastTime = 0; //this will store values from getMs

/*
 * Normally, we'd pull bytes from the USB RX buffer and push to Radio TX buffer until we're out of bytes or until the Radio TX buffer is full
 * However, we're trying to make a temporary copy of some number of bytes, and we don't want to get stuck doing this for a long time
 * For example, if bytes come into the USB RX buffer as fast as we copy them to our temp storage (not sure if possible, but worth thinking about)
 * Of course, we could simultaneously push the copied bytes onto the Radio TX buffer and stop when the buffer was full
 * Which is probably the same as making this value equal to the size of the Radio TX buffer
 * In the interest of simplicity we will just choose a value for this and call it MAX_TX_BYTES
 * Some obvious values emerge: 18 bytes is the value of RADIO_LINK_PAYLOAD_SIZE as defined in radio_link.h, for reasons not clearly documented there
 * Another choice might be the size of a single camera server packet, which would be less than 18 most likely (I make up this format myself)
 * We SHOULD be able to push the data as fast as or faster than the camera server pushes them onto the USB RX buffer
 * Following the note at the top of radio_mac.c, Pololu says that the calibration of the frequency synthesizer takes about 800 us
 * They choose to do this when we go from IDLE to TX, which is something radioLinkChangeChannel (Geoff's code) does
 * The smaller maxTransmittedBytes is, the more times we change channels per bytes of data to be pushed out on all channels
 * The tradeoff is that if it is very large, it takes up a lot of space in memory available to our code
 * as well as increasing the time between individual robots getting the same data based on their position in the chain.
*/

// number of attempts we've made to empty the radio TX queue before changing the channel
uint8 channelChangeTries = 0;

// byte received from USB indicating that the channel should be
// changed to the next received byte
const uint8 CHANNEL_CHANGE_BYTE = 0;

/** Functions *****************************************************************/
void findNextChannel() {
	if (channelStates == 0x0) { sendingState = 0; } //If we detect no live wixels receiving, return to default state every time. Otherwise we'd get stuck in loop below.
	else 
	{
		channelCounter++; //increment the channel the first time
		if (channelCounter > lastChannelIndex) { channelCounter = 0; rollover = 1; } //check for rollover the first time
		while ( (channelStates & ( 0x1 << (channelCounter) ) ) == 0x0 ) //while channelCounter state is a dead state
		{ 
			channelCounter++;
			if (channelCounter > lastChannelIndex) { channelCounter = 0; rollover =1; }
			//increment channel and check for rollover
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

void usbToRadioService()
{
	uint8 byteCounter;			//byte counting for filling tempData or the Radio TX buffer
    
	if (getMs() - lastTime > HEARTBEAT_TIME ) { //If the heartbeat duration has elapsed, for adding back dead channels to the live list (for checking their status)
		lastTime = getMs();
		if ( channelStates != 0xFFFF ) //if there exist dead channels to check
		{
			deadchannelCounter++; //increment the dead channel the first time
			if ( deadchannelCounter > lastChannelIndex ) { deadchannelCounter = 0; } //rollover if needed, first time			
			while ( (channelStates & ( 0x1 << (deadchannelCounter) ) ) == !0x0 ) //while the deadchannelCounter indicates a known live state
			{
				deadchannelCounter++; //increment
				if ( deadchannelCounter > lastChannelIndex ) { deadchannelCounter = 0; } //rollover if needed 
			}
			channelStates = channelStates | ( 0x1 << (deadchannelCounter) ); //add that channel back as live			
		}
	}
	
	if (sendingState==0) //Default state
	{
		byteCounter = 0;
		while ( usbComRxAvailable() && ( byteCounter < MAX_TX_BYTES ) ) //while data is available but tempData is not full
		{
			sendingState = 1; //As soon as I exit this while loop, I should proceed to the send state
			tempData[byteCounter] = usbComRxReceiveByte(); //Put the received byte in tempData
			byteCounter++;
			numTempBytes = byteCounter; //When we exit this loop, numTempBytes will reflect the number of bytes we put in tempData
		}
	}

	else if ( sendingState == 1 ) //Sending state
	{
		byteCounter = 0;
		while ( radioComTxAvailable() && ( byteCounter < numTempBytes ) ) //TX buffer not full, iterate through until numTempBytes
		{
			radioComTxSendByte(tempData[byteCounter]); //Send the byte
			byteCounter++;
		} //Given MAX_TX_BYTES < size of the TX buffer, we'd never expect radioComTxAvailable to be false
		findNextChannel(); //compute the next live channel
		sendingState = 2;
	}
	
	else if (sendingState == 2) //Change channel state
	{
		if ( radioLinkTxQueued() == 0 ) //If we cleared the TX buffer
		{
			radioLinkChangeChannel(channelList[channelCounter]); //execute change to the next channel
			channelChangeTries = 0; //Reset the timeout counter for the next channel
			if (rollover == 1) { rollover = 0; sendingState = 0; } //If we rolled over to beginning, go back to default state
			else { sendingState = 1; } //Otherwise go back to sending state
			LED_RED(0);
		}
		else
		{
			channelChangeTries++;
			if ( channelChangeTries > MAX_CHANNEL_CHANGE_TRIES ) //timeout condition
			{
				channelStates = channelStates & ~ ( 0x1 << (channelCounter - 1) ); //drop the channel (channelCounter-1) from the channel list
				radioLinkTxQueueReset(); //force to clear the Radio TX buffer
			}
			LED_RED(1);
		}
	}
	
    while(radioComRxAvailable() && usbComTxAvailable()) //Radio data is coming in, push it to the USB TX buffer.
	//Note only the wixel first in the list (the channel we're tuned to during default mode) is able to send data back in this scheme
	//I'm keeping this here anyway but it isn't useful to us
    {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}

void robotRadioService()
{
    //robot code could go here, though this code was designed to be loaded onto a PC-USB tethered wixel only
}

void main()
{
    systemInit();    
    radioRegistersInit();
    usbInit();
    radioComRxEnforceOrdering = 0;
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
	    case MODE_TETHERED:  	usbToRadioService(); break;
	    case MODE_UNTETHERED: 	robotRadioService(); break;
	}        
    }
}
