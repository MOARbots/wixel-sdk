#ifndef TRIG_UTILS
#define TRIG_UTILS

#include <cc2511_types.h>

float calculateAngle(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
float standardizeAngle(float angle);
float computeTurn(float robotAngle, float pointAngle);
int turnDirection(float currentAngle, float targetAngle);

#endif
