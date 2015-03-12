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
#include <math.h>
#include <stdio.h>

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

#define A1	0
#define A2	1
#define B1	2
#define B2	3
#define ENABLE	15

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/

uint8 DATA currentMode;
uint8 DATA str[32];

uint32  DATA runUntil;
float   DATA timer;
float   DATA turnAngle;

/** Functions *****************************************************************/

void timer3Init()
{
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

void turnLeft() {
  setDigitalOutput(0,LOW); //Input A1 to motor driver, controls left side
  setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(2,LOW); //Input B1 to motor driver, controls right side
  setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
}

void turnRight() {
  setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(1,LOW); //Input A2 to motor driver, controls left side
  setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(3,LOW); //Input B2 to motor driver, controls right side
}

void goForward() {
  setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(1,LOW); //Input A2 to motor driver, controls left side
  setDigitalOutput(2,LOW); //Input B1 to motor driver, controls right side
  setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
}

void goBackward() {
  setDigitalOutput(0,LOW); //Input A1 to motor driver, controls left side
  setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(3,LOW); //Input B2 to motor driver, controls right side
}

void brake() { // Hard brake.
  setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
}

void sendBackString(char s[]) {
  if (s != '\0') {
    int i = 0;
    if (!radioComTxAvailable()) {
      radioComTxService(); radioLinkTxQueueReset();
    }

    while (s[i] != '\0') {
      if ( radioComTxAvailable()) {
        radioComTxSendByte(s[i]);
      }
      else {
        radioComTxService();
      }
      i++;
    }

    radioComTxService();
  }
}

// x1, y1, rot all properties of robot
// x2, y2 properties of waypoint
float calculateAngle(float x1, float y1, float rot, float x2, float y2) {
  float dX = x2 - x1;

  // TODO this may need to be changed based on april tag's coordinate system
  float dY = y2 - y1;

  float angle = atan2f(dY, dX) * 180 / 3.14159;
  float finalAngle;

  if (angle >= 0) {
    finalAngle = rot - angle;
    sendBackString("Turn LEFT\n");
  } else {
    sendBackString("Turn RIGHT\n");
    finalAngle = rot + angle;
  }

  sprintf(str, "calc angle = %i\n\rfinal angle = %i", (uint8) angle, (uint8) finalAngle);
  sendBackString(str);

  return finalAngle;
}

void angleTest() {
  // Pos angle means turn LEFT
  // Neg angle means turn RIGHT

  // Angle should be 360 (0)
  // Final should be -45
  calculateAngle(0, 0, 45, 1, 0);

  // Angle should be 45
  // Final should be 0
  calculateAngle(0, 0, 45, 1, 1);

  // Angle should be 90
  // Final should be 45
  calculateAngle(0, 0, 45, 0, 1);

  // Angle should be 135
  // Final should be 90
  calculateAngle(0, 0, 45, -1, 1);

  // Angle should be 180 (0)
  // Final should be 135
  calculateAngle(0, 0, 45, -1, 0);

  // Angle should be 225 (-135)
  // Final should be 180 or -180
  calculateAngle(0, 0, 45, -1, -1);

  // Angle should be 270 (-90)
  // Final should be -135
  calculateAngle(0, 0, 45, 0, -1);

  // Angle should be 315 (-45)
  // Final should be -90
  calculateAngle(0, 0, 45, 1, -1);
}


// Estimate how long (in milliseconds) to rotate LEFT to turn this many degrees
// counterclockwise.  Angle should be in [0, 180].  Assumes PWR=100.
float rotateLeft(float angle) {
  return 81.6 + 3.65 * angle;
}

// Estimate how long (in milliseconds) to rotate RIGHT to turn this many degrees
// clockwise. Angle should be in [0, -180]. Assumes PWR=100.
float rotateRight(float angle) {
  return 60.2 - 4.12 * angle;
}

float turn(float angle) {
  float time;

  if (angle >= 0) {
    time = rotateLeft(angle);
    turnLeft();
  } else {
    time = rotateRight(angle);
    turnRight();
  }

  return angle;
}

void turnTest() {
  // angleTest();

  // Calculate 90 degrees, turning to left
  turnAngle = calculateAngle(0, 0, 0, 0, 1);
  timer = turn(turnAngle);

  // Pad the timer just a little bit
  runUntil = (timer * 1.2) +  getMs();
}

void robotRadioService() //runs during UNTETHERED mode, robot behaviors go here
{

  uint8 mybyte; //for the incoming byte storage
  uint8 static set_pwm = 0; //flag that we saw the set pwm symbol 'p'
  uint8 static pwm;


  while(radioComRxAvailable()) {
    mybyte = radioComRxReceiveByte();

    if (set_pwm) {
      pwm = mybyte;
      T3CC0 = T3CC1 = pwm;
      set_pwm = 0;
      continue;
    }

    if (getMs() > runUntil) {
      brake();
    }

    switch(mybyte) { // See http://www.asciitable.com/
      case 0x70: // This is 'p' for 'power'
        set_pwm = 1;
        break;

      case 0x74: //'t' turn test
        turnTest();
        break;

      case 0x77: //'w' forward
        goForward();
        break;

      case 0x73: //'s' backward
        goBackward();
        break;

      case 0x61: //'a' left
        turnLeft();
        break;

      case 0x64: //'d' right
        turnRight();
        break;

      case 0x20: //Space executes a hard brake.
        brake();
        break;
    }
  }
}

void main() {

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
  T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified

  while(1) {
    updateMode();
    boardService();
    updateLeds();
    radioComTxService();
    usbComService();

    switch(currentMode) {
      case MODE_TETHERED:  	  usbToRadioService();  break;
      case MODE_UNTETHERED: 	robotRadioService();  break;
    }
  }

}
