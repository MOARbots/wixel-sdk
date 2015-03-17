#ifndef CONTROL_CAR
#define CONTROL_CAR

#include <wixel.h>

#define A1	0
#define A2	1
#define B1	2
#define B2	3


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


#endif

