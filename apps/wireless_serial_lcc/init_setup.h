#ifndef INIT_SETUP_H
#define INIT_SETUP_H

#define MODE_TETHERED	    0
#define MODE_UNTETHERED		1
//
#define SERIAL_MODE_AUTO        0
#define SERIAL_MODE_USB_RADIO   1
#define SERIAL_MODE_UART_RADIO  2
#define SERIAL_MODE_USB_UART    3

#define ENABLE	15

// pin 
#define A1	0
#define A2	1
#define B1	2
#define B2	3

int32 CODE param_baud_rate = 9600;

uint8 DATA currentMode;

BIT readstate = 0;

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


/** Functions *****************************************************************/
void updateMode() {
  if (usbPowerPresent()) {
    currentMode = MODE_TETHERED;       
  } else {
    currentMode = MODE_UNTETHERED; 
  }
}


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
void putchar(char c) {
    report[reportLength] = c;
    reportLength++;
}

//This sends data over the USB
void sendReportUSB() {
    uint8 bytesToSend;
    // Send the report to USB in chunks.
    if (reportLength > 0) {
        bytesToSend = usbComTxAvailable();
        if (bytesToSend > reportLength - reportBytesSent) {
            // Send the last part of the report.
            usbComTxSend(report+reportBytesSent, reportLength - reportBytesSent);
            reportLength = 0;
        } else {
            usbComTxSend(report+reportBytesSent, bytesToSend);
            reportBytesSent += bytesToSend;
        }
    }
}

#define RO

#ifdef RO
//runs during TETHERED mode, relays info between USB and radio
void usbToRadioService() {
  // Data
  while(usbComRxAvailable() && radioComTxAvailable()) {
    radioComTxSendByte(usbComRxReceiveByte());
  }
  while(radioComRxAvailable() && usbComTxAvailable()) {
    usbComTxSendByte(radioComRxReceiveByte());
  }
}

#else
/*
void usbToRadioService() {
  static int i = 0;
  XDATA uint8 id = 0;
  XDATA int x, y, r;

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
      id = readID();
      x = readX();
      y = readY();
      r = readR();
      if (g_my_id == id) {
        printf("ID: %u, ", id);
        printf("Y: %u, ", y);
        printf("X: %u, ", x);
        printf("R: %u", r);
        putchar('\n'); //note output won't be left aligned properly in screen or gtkterm... TODO a fix for this?
        sendReportUSB();
      }
    }
  }
}
*/
//select usbToRadioService
#endif 
//header
#endif 
