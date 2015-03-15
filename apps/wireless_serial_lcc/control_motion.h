#ifndef CONTROL_MOTION_H
#define CONTROL_MOTION_H


/** Moving Control Flag
 */

/*Bypass*/
const uint16 MOVE_UPDT=1<<0;
/*Direction*/
const uint16 MOVE_LEFT=1<<1; 
const uint16 MOVE_RGHT=1<<2; 
const uint16 MOVE_FWRD=1<<3;
const uint16 MOVE_BWRD=1<<4;
const uint16 MOVE_STOP=1<<5;
/*DURATION: DUR0 is the shortest*/
const uint16 MOVE_DUR0=1<<6; 
const uint16 MOVE_DUR1=1<<7;
const uint16 MOVE_DUR2=1<<8;
const uint16 MOVE_DUR3=1<<9;

/** Mannualy set the duration by prior ;)
 */
int UpdateDuration(uint16 cmd, uint32* duration_ms) {
  if (cmd & MOVE_DUR0) {
    *duration_ms = 10;    return 1;
  } else if (cmd & MOVE_DUR1) {   
    *duration_ms = 30;    return 1;
  } else if (cmd & MOVE_DUR2) {   
    *duration_ms = 90;    return 1;
  } else if (cmd & MOVE_DUR3) {   
    *duration_ms = 200;   return 1;
  }
  return 0;
}

/** Update direction
 */
int UpdateDirection(uint8 cmd) {
  if (cmd == MOVE_LEFT) {
    MoveLeft();     sendBackString("MLft "); return 1;
  } else if (cmd & MOVE_RGHT) {   
    MoveRight();    sendBackString("MRgt "); return 1;
  } else if (cmd & MOVE_FWRD) {   
    MoveForward();  sendBackString("MFwd "); return 1;
  } else if (cmd & MOVE_BWRD) {   
    MoveBackward(); sendBackString("MBwd "); return 1;  
  } else if (cmd & MOVE_STOP) {
    Brake();        sendBackString("MStp "); return 1;
  }
  return 0;
}

/* Control car motion
 */
void MoveUpdate(uint16 cmd) {
  uint32 cur_time = getMs();

  // internel state
  enum {MCTL_STOP=0, MCTL_INIT=1, MCTL_MOVEING=2};
  static uint8  g_mctl_state = 0; 
  static uint32 g_mctl_duetime = 0;
  static uint32 g_mctl_mv_ms = 50; // move 50 ms

  // update motion duration
  if (UpdateDuration(cmd, &g_mctl_mv_ms)) {
    SendBackStr1("Dur", g_mctl_mv_ms);
    //return;
  }
  // update direction
  if (UpdateDirection(cmd)) {
    g_mctl_state = MCTL_INIT;
    //return;
  }
  // update state
  switch (g_mctl_state) {
    case MCTL_STOP: //sendBackString("S0 ");
      Brake();
      break;
    case MCTL_INIT: //sendBackString("S1 ");
      g_mctl_duetime = cur_time + g_mctl_mv_ms;
      g_mctl_state = MCTL_MOVEING;
      break;
    case MCTL_MOVEING: //sendBackString("S2 ");
      if (cur_time > g_mctl_duetime)
          g_mctl_state = MCTL_STOP;
      break;
    default:
      break;
  }
} // MoveUpdate()



#endif
