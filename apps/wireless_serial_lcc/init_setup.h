#ifndef INIT_SETUP_H
#define INIT_SETUP_H

#define ENABLE	15

void timer3Init() {
    // Start the timer in free-running mode and set the prescaler.
    T3CTL = 0b01110000;   // Prescaler 1:8, frequency = (24000 kHz)/8/256 = 11.7 kHz
    //T3CTL = 0b01010000; // Use this line instead if you want 23.4 kHz (1:4)

    // Set the duty cycles to zero.
    T3CC0 = T3CC1 = 0;

    // Enable PWM on both channels.  We choose the mode where the channel
    // goes high when the timer is at 0 and goes low when the timer value
    // is equal to T3CCn.
    T3CCTL0 = T3CCTL1 = 0b00100100;

    // Configure Timer 3 to use Alternative 1 location, which is the default.
    PERCFG &= ~(1<<5);  // PERCFG.T3CFG = 0;

    // Configure P1_3 and P1_4 to be controlled by a peripheral function (Timer 3)
    // instead of being general purpose I/O.
    P1SEL |= (1<<3) | (1<<4);

    // After calling this function, you can set the duty cycles by simply writing
    // to T3CC0 and T3CC1.  A value of 255 results in a 100% duty cycle, and a
    // value of N < 255 results in a duty cycle of N/256.
}

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

   if (!radioLinkConnected()) {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    } else {
        // We have connected.
        if ((now & 0x3FF) <= 20) {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        } else if (dimYellowLed) {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7)==0);
        } else {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred) {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        //dimYellowLed = 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32) {
        dimYellowLed = 0;
    }
}

void Initialize() {
  systemInit();
  usbInit();
  radioComRxEnforceOrdering = 0;
  radioComInit();
  setDigitalOutput(A1,PULLED);
  setDigitalOutput(A2,PULLED);
  setDigitalOutput(B1,PULLED);
  setDigitalOutput(B2,PULLED);
  setDigitalOutput(ENABLE,PULLED);
  timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
  setDigitalOutput(A1,LOW); // Initializing A1, A2, B1, B2 to LOW so the robot doesn't move, but not brake mode
  setDigitalOutput(A2,LOW);
  setDigitalOutput(B1,LOW);
  setDigitalOutput(B2,LOW);
  setDigitalOutput(ENABLE,HIGH); //Standby mode: Motor driver turns off when LOW, on when HIGH

  timeInit();// for uint32 getMS()
}


#endif
