#ifndef __RJCUPDATERS_H__
#define __RJCUPDATERS_H__

void updateLEDs(void) {
	// static BIT dimYellowLed = 0;
 //    static uint16 lastRadioActivityTime;
 //    uint16 now;

    usbShowStatusWithGreenLed();

 //    now = (uint16)getMs();

 //    if (!radioLinkConnected()) {
 //        // We have not connected to another device wirelessly yet, so do a
 //        // 50% blink with a period of 1024 ms.
 //        LED_YELLOW(now & 0x200 ? 1 : 0);
 //    } else {
 //        // We have connected.

 //        if ((now & 0x3FF) <= 20) {
 //            // Do a heartbeat every 1024ms for 21ms.
 //            LED_YELLOW(1);
 //        } else if (dimYellowLed) {
 //            static uint8 DATA count;
 //            count++;
 //            LED_YELLOW((count & 0x7)==0);
 //        } else {
 //            LED_YELLOW(0);
 //        }
 //    }

 //    if (radioLinkActivityOccurred) {
 //        radioLinkActivityOccurred = 0;
 //        dimYellowLed ^= 1;
 //        //dimYellowLed = 1;
 //        lastRadioActivityTime = now;
 //    }

 //    if ((uint16)(now - lastRadioActivityTime) > 32) {
 //        dimYellowLed = 0;
 //    }
}

uint8 updateMode()
{
    if (usbPowerPresent()) {
    	return MODE_TETHERED;       
    } else {
        return MODE_UNTETHERED; 
    }
}

#endif