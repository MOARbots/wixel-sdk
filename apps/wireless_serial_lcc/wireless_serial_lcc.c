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
#include <math.h>

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
XDATA enum {STATE_INVALID = 0, STATE_EXIST = 1, STATE_FOUND = 2};

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
  uint8 i = 0;
  for (i = 0; i < MAX_TARGETS; ++i) {
    targets[i].x = targets[i].y = targets[i].r = 0;
    targets[i].state = STATE_INVALID;
    targets[i].id= 0;
  }
}

/** Update targets list
 */
void UpdateTargets (uint8 id, XDATA uint16 x, XDATA uint16 y, XDATA uint16 r) {
  uint8 i = 0; 
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
void UpdateMyRobot (XDATA uint16 x, XDATA uint16 y, XDATA uint16 r) {
    myrobot.x = x;
    myrobot.y = y;
    myrobot.r = r;
}

/** Find the target from target list, choose the closest one
 *  return 0: not found
 *         1: found it
 */
int GetTargetIdx (uint8* idx, XDATA uint32* squ_dist) {
  uint8 i = 0;
  XDATA uint8 exist = 0;
  XDATA int16 dx = 0;
  XDATA int16 dy = 0;
  XDATA uint32 min_dist = 0;
  XDATA uint32 dist = 0;

  for (i = 0; i < MAX_TARGETS && targets[i].state; ++i) {
    if (targets[i].state == STATE_FOUND)
      continue;
    dx = targets[i].x - myrobot.x; 
    dy = targets[i].y - myrobot.y; 
    dist = dx*dx + dy*dy;
    if (min_dist > dist) {
      *idx = i;
      exist = 1;
      min_dist = dist;
    }
  }
  *squ_dist = min_dist;
  return exist;
}

/** return degree between target and robot
 */
int16 GetDiffOrientation(uint8 tgt_idx) {
  XDATA int16 tgt_x = targets[tgt_idx].x;
  XDATA int16 tgt_y = targets[tgt_idx].y;

  XDATA float dx = tgt_x - myrobot.x;
  XDATA float dy = tgt_y - myrobot.y;
  XDATA float deg = atan2f(dx, dy)*180.f/3.1415926f;
  return deg;
}

/** Rotate car
 *  return 0: finish rotation, diff degree < Threshold_Deg 
 *         1: rotating
 */
int RotateCar(XDATA int16 degree) {
  XDATA uint16 Threshold_Deg = 3;
  XDATA uint16 abs_deg = degree > 0? degree : -degree;
  XDATA uint16 DIRECTION =  degree > 0? MOVE_LEFT: MOVE_RGHT;
  XDATA uint16 DURATION =  0;

  if (abs_deg < Threshold_Deg) 
    return 0;
  else if (abs_deg < 6)
    DURATION = MOVE_DUR0;
  else if (abs_deg < 12)
    DURATION = MOVE_DUR1;
  else
    DURATION = MOVE_DUR2;

  MoveUpdate(DURATION|DIRECTION);
  return 1;
}


/** Update game status and robot motion
 */
void UpdateGame(XDATA uint8 id, XDATA uint16 x, XDATA uint16 y, XDATA uint16 r) {
  //TODO
  //manual setting my_id
  XDATA uint8 my_id = 0;//TODO : set correct id
  XDATA uint8 tgt_idx = 0;
  XDATA int16 diff_orientation = 0;
  XDATA uint32 distance = 0;
  XDATA uint32 Dist_Thld = 10;

  if (id == my_id)
    UpdateMyRobot(x, y, r);
  else 
    UpdateTargets(id, x, y, r);

  // get target idx
  if (!GetTargetIdx(&tgt_idx, &distance)) {
    return;
  }
  // check distance close enough
  // and update list
  if (distance < Dist_Thld) {
    SendBackStr("X");  
    targets[i].state = STATE_FOUND;
  }

  // caculate direction
  diff_orientation = GetDiffOrientation(tgt_idx);

  // update car direction
  if (RotateCar(diff_orientation)) {
    return;
  }
  // move duration
  MoveUpdate(MOVE_FWRD|MOVE_DUR2);
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
