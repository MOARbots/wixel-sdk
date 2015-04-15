/* A library for motor related functions
 * to do: add calibration and speed related functions
 * for example: turn and move functions that are set
 * for a particular speed (degrees/s or meters/s)
 */

#include <motors.h>
#include <wixel.h>

//Define which pins are which motor driver inputs
#define A1	0
#define A2	1
#define B1	2
#define B2	3
#define ENABLE	15

//Set the pull-down resistors
void setMotorsPulled() {
    setDigitalOutput(A1,PULLED);
    setDigitalOutput(A2,PULLED);
    setDigitalOutput(B1,PULLED);
    setDigitalOutput(B2,PULLED);
    setDigitalOutput(ENABLE,PULLED);
}

//enable the motor driver
void enableMotors(BIT standby) {
   setDigitalOutput(ENABLE,standby); //Standby mode: Motor driver turns off when LOW, on when HIGH
}

//Sets the left PWM (pulse width modulation) parameter
void setLeftPWM(uint8 val){
    T3CC0 = val;
}

//Sets the right PWM (pulse width modulation) parameter
void setRightPWM(uint8 val){
    T3CC1 = val;
}

void setLeftDirection(BIT val) {
    if (val == 0) { // go forwards
        setDigitalOutput(A1, HIGH);
        setDigitalOutput(A2, LOW);
    } else if (val == 1) {
        setDigitalOutput(A1, LOW);
        setDigitalOutput(A2, HIGH);
    }
}

void setRightDirection(BIT val) {
    if (val == 0) { // go forwards
        setDigitalOutput(B1, HIGH);
        setDigitalOutput(B2, LOW);
    } else if (val == 1) {
        setDigitalOutput(B1, LOW);
        setDigitalOutput(B2, HIGH);
    }
}

//A coast-type stop
void StopMotors () { 
    setDigitalOutput(A1,LOW);
    setDigitalOutput(A2,LOW);
    setDigitalOutput(B1,LOW);
    setDigitalOutput(B2,LOW);
}

//A hard brake stop
void Brake() {
    setDigitalOutput(A1,LOW);
    setDigitalOutput(A2,LOW);
    setDigitalOutput(B1,LOW);
    setDigitalOutput(B2,LOW);
}

//Sets up motor driver inputs for a left turn type motion (left wheel reverse, right wheel forward)
void Left() {
    setDigitalOutput(A1,LOW); //Input A1 to motor driver, controls left side
    setDigitalOutput(A2,HIGH); //Input A2 to motor driver, controls left side
    setDigitalOutput(B1,LOW); //Input B1 to motor driver, controls right side
    setDigitalOutput(B2,HIGH); //Input B2 to motor driver, controls right side
}

//Sets up motor driver inputs for a right turn type motion (left wheel forward, right wheel reverse)
void Right() {
    setDigitalOutput(A1,HIGH); //Input A1 to motor driver, controls left side
    setDigitalOutput(A2,LOW); //Input A2 to motor driver, controls left side
    setDigitalOutput(B1,HIGH); //Input B1 to motor driver, controls right side
    setDigitalOutput(B2,LOW); //Input B2 to motor driver, controls right side
}

//Sets up motor driver inputs for a forward type motion (both wheels forward)
void Forward() {
    setDigitalOutput(A1,HIGH); //Input A1 to motor driver, controls left side
    setDigitalOutput(A2,LOW); //Input A2 to motor driver, controls left side
    setDigitalOutput(B1,LOW); //Input B1 to motor driver, controls right side
    setDigitalOutput(B2,HIGH); //Input B2 to motor driver, controls right side
}

//Sets up motor driver inputs for a reverse type motion (both wheels reverse)
void Reverse() {
    setDigitalOutput(A1,LOW); //Input A1 to motor driver, controls left side
    setDigitalOutput(A2,HIGH); //Input A2 to motor driver, controls left side
    setDigitalOutput(B1,HIGH); //Input B1 to motor driver, controls right side
    setDigitalOutput(B2,LOW); //Input B2 to motor driver, controls ri
}

//initialization function
void motorsInit() {
    setMotorsPulled();
    StopMotors(); //avoid moving when you start the routine
    enableMotors(1);
    setLeftPWM(255);
    setRightPWM(255);
}
