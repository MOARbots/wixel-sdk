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

/** ReadMe:
 *  1. set the correct id (based on tag number) to g_my_id
 *  2. signal symbol 's' to start it.
 */


/** Global Variables **********************************************************/
static uint8 g_my_id = 7;
static uint8 g_cbk_d = 0;
static uint8 g_cbk_g = 1;
static uint8 g_cbk_f = 0;
static uint8 g_cbk_s = 0;

#define MAX_TARGETS 32
#define INPUT_MAX_X 640
#define INPUT_MAX_Y 480

#define DEBUGx

XDATA enum {STATE_INVALID = 0, STATE_EXIST = 1, STATE_FOUND = 2};

typedef struct Status{
  int16 x, y, r; // x, y, orientation
  int8 state; // STATE_INVALID = 0, STATE_EXIST = 1, STATE_FOUND = 2
  int8 id;
} Status;


/** Functions *****************************************************************/
// these header are implementations
// DON'T Reorder these headers
#include "init_setup.h"
#include "radio_str.h"
#include "control_motion.h"

#ifdef DEBUG
#include "fake_data.h"
#endif

//uint8 mybyte: for the incoming byte storage
void robotRadioService(uint8 mybyte) //runs during UNTETHERED mode, robot behaviors go here
{
  switch(mybyte) {
    case 0x70://'p', change pwm
      break;
    case 0x77://'w' //MoveUpdate(MOVE_FWRD);
      break; 
    case 0x61://'a' //MoveUpdate(MOVE_LEFT);
      break; 
    case 0x73://'s' //MoveUpdate(MOVE_BWRD);
      if (g_cbk_s == 0) {
        g_cbk_s = 1; SendBackStr("\nFlag S on\n");
      } else {
        g_cbk_s = 0; SendBackStr("\nFlag S off\n");
      }
      break; 
    case 0x64://'d' // get target and difference deg
      g_cbk_d = 1; //MoveUpdate(MOVE_RGHT);
      break; 
    case 0x20://Space executes a hard brake
      //Brake(); //MoveUpdate(MOVE_STOP);
      break;
    case 0x66://'f'
      g_cbk_f = 1; //MoveUpdate(MOVE_DUR0);
      break; 
    case 0x67://'g'
      if (g_cbk_g == 0) {
        g_cbk_g = 1; SendBackStr("\nFlag G on\n");
      } else {
        g_cbk_g = 0; SendBackStr("\nFlag G off\n");
      }
      break;
    case 0x68://'h'
      break;
  }
}

/******/
/** Target List
 */
XDATA Status targets[MAX_TARGETS];
XDATA Status myrobot;

void InitStatus() {
  // targts
  uint16 i = 0;
  for (i = 0; i < MAX_TARGETS; ++i) {
    targets[i].state = STATE_INVALID;
    targets[i].id= 0;
    targets[i].x = targets[i].y = targets[i].r = 0;
  }
  // robot
  myrobot.x = 0;
  myrobot.y = 0;
  myrobot.state = STATE_INVALID;
  myrobot.id = g_my_id;
}

/** Update targets list
 *  return 0 if no target updated
 *         1 otherwise
 */
int UpdateTargets (int8 id, int16 x, int16 y, int16 r) {
  int retv = 0;
  int state = 0; 
  if (id >= MAX_TARGETS) {
    SendBackStr("-ERROR Invaid Target Id");
    return 0;
  }
  state = targets[id].state;

  if (state == STATE_INVALID) {
    targets[id].state = STATE_EXIST;
    targets[id].x = x;
    targets[id].y = y;
    targets[id].r = r;
    if (g_cbk_g) SendBackStr3("UTgtIXY", id, x, y);  
    retv = 1;
  } else 
  if (state == STATE_EXIST) {
    if (g_cbk_g) {SendBackStr("UTgtExt");}
    retv = 0; 
  } else 
  if (state == STATE_FOUND) {
    retv = 0;
  }
  return retv;
}

/** Update robot current status 
 */
XDATA uint32 upd = 0;
void UpdateRobot (int16 x, int16 y, int16 r) {
  myrobot.state = STATE_EXIST;
  myrobot.x = x;
  myrobot.y = y;
  myrobot.r = r;
}

/** Find the target from target list, choose the closest one
 *  return 0: not found
 *         1: found it
 */
XDATA int g_approach_id = MAX_TARGETS + 1;

int GetTargetIdx (int8* idx, int32* squ_dist) {
  int8 i = 0;
  XDATA uint8 exist = 0;
  XDATA int32 dx = 0;
  XDATA int32 dy = 0;
  XDATA int32 min_dist = 1700;
  XDATA int32 dist = 0;
  /*
  XDATA int32 x2 = 0;
  XDATA int32 y2 = 0;
  XDATA float x3 = 0;
  XDATA float y3 = 0;
  */
  for (i = 0; i < MAX_TARGETS; ++i) {
    if (targets[i].state == STATE_FOUND)
      continue;
    if (targets[i].state == STATE_INVALID)
      continue;
    if (targets[i].state == STATE_EXIST) {
      dx = targets[i].x - myrobot.x; 
      dy = targets[i].y - myrobot.y; 
      if (dx < 0) dx = -dx;
      if (dy < 0) dy = -dy;
      dist  = sqrtf(dx*dx + dy*dy);
      //dist = dx+dy;
      if (min_dist > dist) {
        *idx = i;
        exist = 1;
        min_dist = dist;
        if (g_cbk_g) {
          SendBackStr2("mXY", myrobot.x, myrobot.y);  
          SendBackStr2("dXY", dx, dy);  
          //x3 = dx*dx; y3 = dy*dy; x2 = x3; y2 = x3;
          //SendBackStr3("DXY", x2, y2, sqrtf(x2+y2));  
          SendBackStr3("tXYDt", targets[i].x, targets[i].y, min_dist);
        } 
      }
    }
  }
  *squ_dist = min_dist;

  // debug: show the approaching target id at first time.
  // g_approach_id prevent SendBackStr1 every time.
  if (exist && g_approach_id != targets[*idx].id) {
    g_approach_id = targets[*idx].id;
    if (g_cbk_g) SendBackStr1("ApprTgt", targets[*idx].id);  
  }
  return exist;
}

/** return degree between target and robot
 */
int16 GetDiffOrientation(uint8 tgt_idx) {
  XDATA float dx = targets[tgt_idx].x - myrobot.x;
  XDATA float dy = targets[tgt_idx].y - myrobot.y;
  //TODO it wierd that the order is (dy,dx) instead of (dx,dy)
  XDATA float deg = atan2f(dy,dx)*180.f/3.1415926f;
  if (deg < 0)
    deg += 360;
  deg = myrobot.r - deg;
  if (deg > 180)
    deg = 360 - deg;
  return deg;
}

/** Rotate car
 *  return 0: finish rotation, diff degree < ANGLE_0
 *         1: rotating
 */
#define ANGLE_0  10 
#define ANGLE_1  30
#define ANGLE_2  50

#define DISTANCE_CLOSE_0 20 
#define DISTANCE_CLOSE_1 90 
#define DISTANCE_CLOSE_2 180 

int RotateCar(XDATA int16 degree) {
  XDATA uint16 abs_deg = degree > 0? degree : -degree;
  XDATA uint16 DIRECTION = 0; 
  XDATA uint16 DURATION  = 0;
  // Left or Right
  if (degree < 0) {
    if (abs_deg < 180) {
      DIRECTION = MOVE_LEFT;
    } else {
      DIRECTION = MOVE_RGHT;
    }
  } else {
     if (abs_deg < 180) {
      DIRECTION = MOVE_RGHT;
    } else {
      DIRECTION = MOVE_LEFT;
    }
  }

  // Moving Duration
  if (abs_deg > 180)
    abs_deg = 360 - abs_deg;

  // Update duration
  if (abs_deg < ANGLE_0) 
    return 0;
  else if (abs_deg < ANGLE_1)
    DURATION = MOVE_DUR0;
  else if (abs_deg < ANGLE_2)
    DURATION = MOVE_DUR1;
  else
    DURATION = MOVE_DUR2;

  MoveUpdate(DURATION|DIRECTION);
  return 1;
}


/** Bandit to shadow issue.
 *  Detect delta time and force the car to move if not received data for a while.
 */
XDATA uint32 g_shw_t0 = 0;
void ForceShadowMove() {
  XDATA int32 cur_t = getMs();
  XDATA int32 d_t = cur_t - g_shw_t0;

  if (d_t > 800/*0.8 sec*/) {
    MoveUpdate(MOVE_DUR2|MOVE_BWRD);
    g_shw_t0 = cur_t;
  } 
}


int cc = 0;


/** Update game status and robot motion
 */
void UpdateGame(int16 id, int16 x, int16 y, int16 r) {
  XDATA int8 tgt_idx = 0;
  XDATA int16 diff_orientation = 0;
  XDATA int32 distance = 0;
  XDATA int32 Dist_Thld = 30;

  if (g_cbk_g) {SendBackStr("\n");}

#ifdef DEBUG
  if(!ReadFakeData(&id, &x, &y, &r))
    ReadFakeData2(&id ,&x, &y, &r);
#else
  y = INPUT_MAX_Y - y;
  if (y < 0) SendBackStr("\n --- WIERD COORDINATE Y ---\n");
#endif
  //if (cc++%10==0) { cc = 0;  SendBackStr3("\nFbk", id, x, y); }

  // because ReadID could be invalid
  if (id==0) {
    return;
  }
  // update positions
  if (id == g_my_id) {
    UpdateRobot(x, y, r);
    g_shw_t0 = getMs();// bandit to deal with shadow issue
    if (g_cbk_g) SendBackStr3("RbXYR", x, y, r);  
  } else {
    if (!UpdateTargets(id, x, y, r)) {
      return;
    }
  }
  // make sure the robot exists
  if (myrobot.state != STATE_EXIST)
    return;
  // get target idx
  if (!GetTargetIdx(&tgt_idx, &distance)) {
    if (g_cbk_g) {SendBackStr("Tgt None");}
    return;
  }
  // check goal
  if (distance < Dist_Thld) {
    SendBackStr("-X");  
    SendBackStr1("TgtFund", distance);  
    targets[tgt_idx].state = STATE_FOUND;
    return;
  }

  // caculate direction
  diff_orientation = GetDiffOrientation(tgt_idx);
  if (g_cbk_g) {SendBackStr3("IDtDg", tgt_idx, distance, diff_orientation);}

  // update car direction
  if (RotateCar(diff_orientation)) {
    if (g_cbk_g) {SendBackStr("+R");}
    return;
  }
  // update move straight duration
  if (distance < DISTANCE_CLOSE_0 ) {
    MoveUpdate(MOVE_FWRD|MOVE_DUR1);
  } else if (distance < DISTANCE_CLOSE_1 ) {
    MoveUpdate(MOVE_FWRD|MOVE_DUR2);
  } else if (distance < DISTANCE_CLOSE_2 ) {
    MoveUpdate(MOVE_FWRD|MOVE_DUR3);
  } else {
    MoveUpdate(MOVE_FWRD|MOVE_DUR4);
  }

  if (g_cbk_g) {SendBackStr("+M");}
}

//This mode is active when the USB is plugged in
//Test Streaming Data to USB. Data is received over the radio.
//If the packet is properly formatted (111111 header, 5 bit ID, 10 bit Y pos, 10 bit X pos, 9 bit rotation, total 5 bytes)
//Then the Wixel will send this back via the PC it is connected to via USB.
int can_read_packet = 0;
int pk_i = 0;
void UpdatePacketService() {
  uint8 byte = 0;
  if (!readstate) { //the idle state	
    if (radioComRxAvailable()) {
      byte = radioComRxReceiveByte();
      if ( checkHeader(byte) ) { 
        readstate=1; 
      } else {
        robotRadioService(byte);
      }
    }
  } else { //readstate
    if ( pk_i + 1 < QTupleSize ) { //packet not yet full
      if (radioComRxAvailable()) { //if byte available
        packet.bytes[pk_i+1] = radioComRxReceiveByte(); //take it off the receiving buffer
        pk_i = pk_i+1;
      }
    } else { //We filled up one packet
      readstate = 0; //return to idle state next loop
      pk_i=0;
      can_read_packet = 1;
    }
  }
}

void main()
{
  Initialize();
  T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified
  InitStatus();

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
        UpdatePacketService();
#ifndef DEBUG
        // fire
        if (!g_cbk_s) {
          break;
        }
        if (can_read_packet) {
#else
        if (g_cbk_f) {
#endif
          UpdateGame(readID(&packet), readX(&packet), readY(&packet), readR(&packet));
          can_read_packet = 0;
        }
        MoveUpdate(MOVE_UPDT);
        ForceShadowMove();// bandit to deal with shadow issue

        //debug flag
        g_cbk_f = 0;
        if (g_cbk_d) {
          MoveUpdate(MOVE_DUR1|MOVE_FWRD);
          g_cbk_d = 0;
        }
        break;
      default: break;
    }
  } //while
}//main
