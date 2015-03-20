#include <trig_utils.h>
#include <math.h>
#include <float.h>

// Calculate angle between two points P1 and P2
// Consider P1 as the robot
// Consider P2 as the waypoint
float calculateAngle(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
  int dX = (int) (x2 - x1);
  // Y based off opposite coordinate system
  int dY = (int) (y2 - y1);

  float angle = atan2f((float) dY, (float) dX) * (180.0 / 3.14159);
  // return angle;
  return standardizeAngle(angle);
}

// Standardize a given angle to between 0 and 360
float standardizeAngle(float angle) {
  while (angle < 0) {
    angle += 360.0;
  }

  while (angle > 360) {
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

// TODO: Make sure this is correct
// Return 1 for a left turn, 0 for a right turn - should make an enum
int turnDirection(float currentAngle, float targetAngle) {
  currentAngle = standardizeAngle(currentAngle);
  targetAngle = standardizeAngle(targetAngle);

  if (targetAngle - currentAngle <= 0) {
    return 1;
  }

  return 0;
}
