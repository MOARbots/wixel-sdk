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
#include <uart1.h>
#include <stdio.h>

// these header are implementations
#include "control_car.h"
#include "init_setup.h"

/** Parameters ****************************************************************/
#define MODE_TETHERED	        0
#define MODE_UNTETHERED		1

#define SERIAL_MODE_AUTO        0
#define SERIAL_MODE_USB_RADIO   1
#define SERIAL_MODE_UART_RADIO  2
#define SERIAL_MODE_USB_UART    3

int32 CODE param_baud_rate = 9600;

/** Global Variables **********************************************************/
uint8 DATA currentMode;

//uint8 DATA currentSerialMode;

/** Functions *****************************************************************/
void updateMode() {
  if (usbPowerPresent()) {
    currentMode = MODE_TETHERED;       
  } else {
    currentMode = MODE_UNTETHERED; 
  }
}

void usbToRadioService() //runs during TETHERED mode, relays info between USB and radio
{
    // Data
    while(usbComRxAvailable() && radioComTxAvailable()) {
        radioComTxSendByte(usbComRxReceiveByte());
    }

    while(radioComRxAvailable() && usbComTxAvailable()) {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}


void UpdateWayPoints() {

}

void sendBackString(char s[]) { 
  int i = 0; 
  if (currentMode != SERIAL_MODE_USB_UART && s != NULL) { 
    if (!radioComTxAvailable()) { radioComTxService(); radioLinkTxQueueReset();  } 
    while (s[i] != '\0') { 
      if ( radioComTxAvailable() ) { radioComTxSendByte(s[i]);  } 
      else { radioComTxService();  } 
      i++; 
    } 
    radioComTxService(); 
  } 
}

//
// g_mctl_move: Moving Control Flag
// 0: no move, 
// 1: set moving properties, 
// 2: moving
//

enum {MOVE_LEFT=0, MOVE_RGHT=1, MOVE_FWRD=2, MOVE_BWRD=3, MOVE_STOP=4, MOVE_UPDT=5};
enum {MCTL_STOP=0, MCTL_INIT=1, MCTL_MOVEING=2};
static uint8  g_mctl_state = 0; 
static uint32 g_mctl_duetime = 0;
static uint32 g_mctl_mv_ms = 50; // move 50 ms

void MoveUpdate(uint8 cmd) {
  uint32 cur_time = getMs();

  // move direction
  switch (cmd) {
    case MOVE_LEFT:
      sendBackString("MLft ");
      g_mctl_state = MCTL_INIT;
      MoveLeft(); 
      break;
    case MOVE_RGHT:
      sendBackString("MRgt ");
      g_mctl_state = MCTL_INIT;
      MoveRight(); 
      break;
    case MOVE_FWRD:
      sendBackString("MFwd ");
      g_mctl_state = MCTL_INIT;
      MoveForward(); 
      break;
    case MOVE_BWRD:
      sendBackString("MBwd ");
      g_mctl_state = MCTL_INIT;
      MoveBackward(); 
      break;
    case MOVE_STOP:
      sendBackString("MStp ");
      Brake();
      break;
    default:
      break;
  }

  // update state
  switch (g_mctl_state) {
    case MCTL_STOP:
      //sendBackString("S0 ");
      break;
    case MCTL_INIT:
      //sendBackString("S1 ");
      g_mctl_duetime = cur_time + g_mctl_mv_ms;
      g_mctl_state = MCTL_MOVEING;
      break;
    case MCTL_MOVEING:
      //sendBackString("S2 ");
      if (cur_time > g_mctl_duetime) {
          g_mctl_state = MCTL_STOP;
          Brake();
      }
      break;
    default:
      break;
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
      T3CC0 = T3CC1 = pwm;
      set_pwm = 0;
      continue;
    }

    switch(mybyte) {
      case 0x70://'p', change pwm
        set_pwm = 1;
        break;
      case 0x77://'w'
        MoveForward();
        MoveUpdate(MOVE_FWRD);
        break; 
      case 0x61://'a'
        MoveLeft();
        MoveUpdate(MOVE_LEFT);
        break; 
        break; 
      case 0x73://'s'
        MoveBackward();
        MoveUpdate(MOVE_BWRD);
        break; 
      case 0x64://'d'
        MoveRight();
        MoveUpdate(MOVE_RGHT);
        break; 
      case 0x20://Space executes a hard brake
        Brake();
        MoveUpdate(MOVE_STOP);
        break;
      case 0x65://'e'
        MoveUpdate(MOVE_LEFT);
        break; 
    }
    //
  }
}

void main()
{
  Initialize();
  T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified

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

    //lc: update state
    MoveUpdate(MOVE_UPDT);
  }
}
