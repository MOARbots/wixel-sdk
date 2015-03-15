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
#include <packet.h>

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

/** Functions *****************************************************************/

// these header are implementations
#include "control_car.h"
#include "init_setup.h"
#include "radio_str.h"
#include "control_motion.h"

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
        MoveUpdate(MOVE_FWRD);
        break; 
      case 0x61://'a'
        MoveUpdate(MOVE_LEFT);
        break; 
        break; 
      case 0x73://'s'
        MoveUpdate(MOVE_BWRD);
        break; 
      case 0x64://'d'
        MoveUpdate(MOVE_RGHT);
        break; 
      case 0x20://Space executes a hard brake
        Brake();
        MoveUpdate(MOVE_STOP);
        break;
      case 0x65://'e'
        MoveUpdate(MOVE_LEFT);
        break; 
      case 0x66://'f'
        MoveUpdate(MOVE_DUR0);
        break; 
      case 0x67://'g'
        MoveUpdate(MOVE_DUR1);
        break;
      case 0x68://'h'
        MoveUpdate(MOVE_DUR2);
        break;
    }
  }
}


/******/
enum {STATE_INVALID = 0, STATE_EXIST = 1, STATE_FOUND = 2};

typedef struct Status{
  uint16 x, y, r; // x, y, orientation
  uint8 state; // STATE_INVALID = 0, STATE_EXIST = 1, STATE_FOUND = 2
  uint8 id;
} Status;

/** Target List
 */
#define MAX_TARGETS 32
XDATA Status targets[MAX_TARGETS];
XDATA Status myrobot;

void InitTargets() {
  int i = 0;
  for (i = 0; i < MAX_TARGETS; ++i) {
    targets[i].x = targets[i].y = targets[i].r = 0;
    targets[i].state = STATE_INVALID;
    targets[i].id= 0;
  }
}

/** Update targets list
 */
void UpdateTargets (uint8 id, uint16 x, uint16 y, uint16 r) {
  int i = 0; 
  uint8 state = STATE_INVALID;
  for (i = 0; i < MAX_TARGETS && targets[i].state; ++i) {
    if (targets[i].id == id) {
      state = targets[i].state; 
      break;    
    }
  }
  if (state == STATE_INVALID && i < MAX_TARGETS) {
    targets[i].state = STATE_EXIST;
    targets[i].id = id;
    targets[i].x = x;
    targets[i].y = y;
    targets[i].r = r;
  }
}

/** Update robot current status 
 */
void UpdateMyRobot (uint16 x, uint16 y, uint16 r) {
    myrobot.x = x;
    myrobot.y = y;
    myrobot.r = r;
}

int GetTargetIdx(uint8* idx) {
  uint8 i = 0;
  uint8 exist = 0;
  uint32 dist = 0;
  uint32 min_dist = 0xffffffff;
  int16 dx = 0;
  int16 dy = 0;

  for (i = 0; i < MAX_TARGETS && targets[i].state; ++i) {
    dx = targets[i].x - myrobot.x; 
    dy = targets[i].y - myrobot.y; 
    dist = dx*dx + dy*dy;
    if (min_dist > dist) {
      *idx = i;
      exist = 1;
      //min_dist = dist;
    }
  }
  return exist;
}

void Set() {
  int a = 0;
}


void UpdateGame(uint8 id, uint16 x, uint16 y, uint16 r) {
  //manual setting my_id
  int my_id = 0;
  int direction = 0;
  int distance = 0;
  uint8 tgt_idx = 0;

  if (id == my_id)
    UpdateMyRobot(x, y, r);
  else 
    UpdateTargets(id, x, y, r);

  // get target idx
  if (!GetTargetIdx(&tgt_idx)) {
    return;
  }

  // caculate direction and distance

  // set my move direction

  // set my move duration

}


void main()
{
  Initialize();
  T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified

  while(1) {
    updateMode();
    boardService();
    updateLeds();  
    radioComTxService();
    usbComService();

    switch(currentMode) {
      case MODE_TETHERED:  	
        usbToRadioService();  
        break;
      case MODE_UNTETHERED: 
        robotRadioService(); 
        UpdateGame(readID(), readX(), readY(), readR());
        MoveUpdate(MOVE_UPDT);
        break;
      default: break;
    }
  } //while
}
