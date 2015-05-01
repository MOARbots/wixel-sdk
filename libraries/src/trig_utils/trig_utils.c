#include <trig_utils.h>
#include <math.h>
#include <float.h>

//Compute the distance between two points
float distance (uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
    return sqrtf( (x2-x1)*(x2-x1)  + (y2-y1)*(y2-y1) );
}

//Compute the distance between two points
uint16 distanceInt(int x1, int y1, int x2, int y2) {
  uint16 dist = (uint16) sqrtf( (x2-x1)*(x2-x1)  + (y2-y1)*(y2-y1) );
  return dist;
}

// Calculate angle between two points P1 and P2
// Consider P1 as the robot
// Consider P2 as the waypoint
float calculateAngle(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
  int dX = (int) (x2 - x1);
  // Y based off opposite coordinate system
  int dY = (int) (y2 - y1);

  float angle = atan2f((float) dY, (float) dX) * (180.0 / 3.14159);
  //return angle;
  return standardizeAngle(-angle); //Note negative sign, to flip output over the x axis
  //This is because camera coordinates have Y=0 at the top of the frame, and Y=MAX at the bottom of the frame
}

// Standardize a given angle to between 0 and 360
float standardizeAngle(float angle) {
  while (angle < -180.0) {
    angle += 360.0;
  }

  while (angle > 180.0) {
    angle -= 360.0;
  }

  return angle;
}

// TODO: Make sure this is correct
// Compute the amount the robot has to turn
float computeTurn(float robotAngle, float pointAngle) {
  float computedAngle = pointAngle - robotAngle;
  computedAngle = standardizeAngle(computedAngle);

  return computedAngle;
}

// Return 0 for a left turn, 1 for a right turn - should make an enum
BIT turnDirection(float currentAngle, float targetAngle) {
  float diff = computeTurn(currentAngle,targetAngle);
  if (diff <= 0) {
    return 1;
  }

  return 0;
}
