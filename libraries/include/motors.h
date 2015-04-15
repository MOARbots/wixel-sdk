#ifndef MOTORS_H
#define MOTORS_H
/* A library for motor related functions
 * 
 */

#include <cc2511_types.h>

void setMotorsPulled();
void enableMotors(BIT standby);
void setLeftPWM(uint8 val);
void setRightPWM(uint8 val);
void setLeftDirection(BIT val);
void setRightDirection(BIT val);
void StopMotors ();
void Brake();
void Left();
void Right();
void Forward();
void Reverse();
void motorsInit();

#endif
