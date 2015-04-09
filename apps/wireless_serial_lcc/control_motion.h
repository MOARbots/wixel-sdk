#ifndef CONTROL_MOTION_H
#define CONTROL_MOTION_H

#include <wixel.h>

void MoveForward() {
  setDigitalOutput(A1,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(A2,LOW); //Input A2 to motor driver, controls left side
  setDigitalOutput(B1,LOW); //Input B1 to motor driver, controls right side
  setDigitalOutput(B2,HIGH); //Input B2 to motor driver, controls right side
}

void MoveBackward() {
  setDigitalOutput(A1,LOW); //Input A1 to motor driver, controls left side
  setDigitalOutput(A2,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(B1,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(B2,LOW); //Input B2 to motor driver, controls right side
}

void MoveRight() {
  setDigitalOutput(A1,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(A2,LOW); //Input A2 to motor driver, controls left side
  setDigitalOutput(B1,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(B2,LOW); //Input B2 to motor driver, controls right side
}

void MoveLeft() {
  setDigitalOutput(A1,LOW); //Input A1 to motor driver, controls left side
  setDigitalOutput(A2,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(B1,LOW); //Input B1 to motor driver, controls right side
  setDigitalOutput(B2,HIGH); //Input B2 to motor driver, controls right side
}

void Brake() {
  setDigitalOutput(A1,HIGH); //Input A1 to motor driver, controls left side
  setDigitalOutput(A2,HIGH); //Input A2 to motor driver, controls left side
  setDigitalOutput(B1,HIGH); //Input B1 to motor driver, controls right side
  setDigitalOutput(B2,HIGH); //Input B2 to motor driver, controls right side
}



/** Moving Control Flag
 */

/*Bypass*/
XDATA const uint16 MOVE_UPDT=1<<0;
/*Direction*/
XDATA const uint16 MOVE_LEFT=1<<1; 
XDATA const uint16 MOVE_RGHT=1<<2; 
XDATA const uint16 MOVE_FWRD=1<<3;
XDATA const uint16 MOVE_BWRD=1<<4;
XDATA const uint16 MOVE_STOP=1<<5;
/*DURATION: DUR0 is the shortest*/
XDATA const uint16 MOVE_DUR0=1<<6; 
XDATA const uint16 MOVE_DUR1=1<<7;
XDATA const uint16 MOVE_DUR2=1<<8;
XDATA const uint16 MOVE_DUR3=1<<9;
XDATA const uint16 MOVE_DUR4=1<<10;

/** Mannualy set the duration by prior ;)
 */
int UpdateDuration(uint16 cmd, uint32* duration_ms) {
  *duration_ms = 0;

  if (cmd & MOVE_DUR0) {
    *duration_ms = 10;    
    //debug
    if (g_cbk_g) { SendBackStr1("Dur", *duration_ms); }
    return 1;
  } else if (cmd & MOVE_DUR1) {   
    *duration_ms = 20;    
    //debug
    if (g_cbk_g) { SendBackStr1("Dur", *duration_ms); }
    return 1;
  } else if (cmd & MOVE_DUR2) {   
    *duration_ms = 30;    
    //debug
    if (g_cbk_g) { SendBackStr1("Dur", *duration_ms); }
    return 1;
  } else if (cmd & MOVE_DUR3) {   
    *duration_ms = 50;   
    //debug
    if (g_cbk_g) { SendBackStr1("Dur", *duration_ms); }
    return 1;
  } else if (cmd & MOVE_DUR4) {   
    *duration_ms = 70;   
    //debug
    if (g_cbk_g) { SendBackStr1("Dur", *duration_ms); }
    return 1;
  }
  return 0;
}

/** Update direction
 */
int UpdateDirection(uint8 cmd) {
  if (cmd & MOVE_LEFT) {
    //debug
    if (g_cbk_g) { SendBackStr("MLf"); }
    MoveLeft(); 
    return 1;
  } else if (cmd & MOVE_RGHT) {   
    //debug
    if (g_cbk_g) { SendBackStr("MRt"); }
    MoveRight();    
    return 1;
  } else if (cmd & MOVE_FWRD) {   
    //debug
    if (g_cbk_g) { SendBackStr("MFw"); }
    MoveForward();   
    return 1;
  } else if (cmd & MOVE_BWRD) {   
    //debug
    if (g_cbk_g) { SendBackStr("MBw");}
    MoveBackward();  
    return 1;  
  } else if (cmd & MOVE_STOP) {
    //debug
    if (g_cbk_g) { SendBackStr("MSt"); }
    Brake();         
    return 1;
  }
  return 0;
}

/* Control car motion
 */
void MoveUpdate(uint16 cmd) {
  XDATA uint32 cur_time = getMs();

  // internel state
  XDATA enum {MCTL_STOP=0, MCTL_INIT=1, MCTL_MOVEING=2};
  static uint8  g_mctl_state = 0; 
  static uint32 g_mctl_duetime = 0;
  static uint32 g_mctl_mv_ms = 20; // move 20 ms

  // update motion duration
  if (UpdateDuration(cmd, &g_mctl_mv_ms)) {
    g_mctl_state = MCTL_INIT;
    //return;
  }
  // update direction
  if (UpdateDirection(cmd)) {
    g_mctl_state = MCTL_INIT;
    //return;
  }
  // update state
  switch (g_mctl_state) {
    case MCTL_STOP:
      Brake();
      break;
    case MCTL_INIT: 
      g_mctl_duetime = cur_time + g_mctl_mv_ms;
      g_mctl_state = MCTL_MOVEING;
      break;
    case MCTL_MOVEING: 
      if (cur_time > g_mctl_duetime)
          g_mctl_state = MCTL_STOP;
      break;
    default:
      break;
  }
} // MoveUpdate()

#endif

