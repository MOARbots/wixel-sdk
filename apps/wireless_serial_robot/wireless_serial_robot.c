/* Wireless_serial_robot app: wireless_serial + PWM on timer 3
 * When plugged into battery power but not USB connected (MODE_UNTETHERED)
 * this app will respond to key presses w,a,s,d, and Space to drive around the robot.
 * Program another Wixel with a wireless_serial app on the same channel
 * Use a serial communication program like PuTTY or screen to send the keypresses
 * to control the robot.
 */

/** Dependencies **************************************************************/
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <timer.h>
#include <motors.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1



int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;

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

void usbToRadioService() //runs during TETHERED mode, relays info between USB and radio
{
    // Data
    while(usbComRxAvailable() && radioComTxAvailable())
    {
        radioComTxSendByte(usbComRxReceiveByte());
    }

    while(radioComRxAvailable() && usbComTxAvailable())
    {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}

void robotRadioService() //runs during UNTETHERED mode, robot behaviors go here
{
    
    uint8 mybyte; //for the incoming byte storage
    uint8 static set_pwm = 0; //flag that we saw the set pwm symbol 'p'
    uint8 static pwm;

    while(radioComRxAvailable()){
    	mybyte = radioComRxReceiveByte();

        if (set_pwm) {
            pwm = mybyte;
            setLeftPWM(pwm);
	    setRightPWM(pwm);
            set_pwm = 0; //restore flag to default state
            continue;
        }

    	switch(mybyte) {
            case 0x70:
                set_pwm = 1; //set flag to receive next input byte as PWM value
            break;
            case 0x77: //char 'w'
        	Forward();	
    		break;
            case 0x61: //char 'a'
        	Left();
    		break;
            case 0x73: //char 's'
        	Reverse();
    		break;
            case 0x64: //char 'd'
        	Right();
    		break;
            case 0x20: //char ' '
        	Brake();
    		break;
            }
    }
}

void main()
{
    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0;
    radioComInit();
    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    motorsInit();

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
