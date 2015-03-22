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

    	switch (currentMode) {
	    	case MODE_TETHERED: computerLink(); break;
	    	case MODE_UNTETHERED: robot(); break;
	    }
    }
}