/* Wireless_serial_robot app: wireless_serial + PWM on timer 3
 * When plugged into battery power (MODE_UART_RADIO) and NOT connect via USB
 * this app will respond to key presses w,a,s,d, and Space to drive around the robot.
 * Program another Wixel with a wireless_serial app on the same channel
 * Use a serial communication program like PuTTY or screen to send the keypresses
 * to control the robot.
 */

/** Dependencies **************************************************************/
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <wixel.h>
#include <usb.h>
#include <usb_com.h>
#include <radio_com.h>
#include <radio_link.h>
#include <uart1.h>
#include <packet.h> 
#include <float.h>

// NOTE: sometimes communications get overwhelmed, so you might want to play around with these.
//       occassionally a few odd chars print or are skipped, but the messages are generally readable.

// These are used in #ifdef so values are not needed.
// Turn off 'reportInfo' to cut down on what is sent back to the user.
#define reportInfo
// Unset 'transmitBack' if NOTHING should be sent back.
#define transmitBack
// If set, will use short info messages in an attempt to not overload communications as much.
// NOTE: got some odd behavior when I commented-out the next line and have not yet figured out the bug.
#define shortStrings

// Use 48 for digits (ie, Tag1, Tag2, etc).
//#define offsetForName     48
// Use 64 for cap letters (ie, TagA, TagBm etc).  This seems visually better since so many numbers already print.
//#define offsetForName     64
#define offsetForName       48

// Play around with various speeds.
// #define powerForGoingForward        90
// #define powerForGoingForwardSlowly  80
// #define powerForTurning             80
// #define powerForTurningSlowly       75
#define powerForGoingForward        125
#define powerForGoingForwardSlowly  100
#define powerForTurning              80
#define powerForTurningSlowly        75


// What are the units in Shira's setup?  PIXELS!  (Angles are in degrees.)
// Seems the AprilTags are about 25 pixels in size.  Thjis is used to decide when have 'visited' a given tag.
#define minDistanceToBeCloseEnoughToAprilTag   15

// If too small, will oscillate trying to get lower than ± this.  If within this range, will drive forward until angle to target becomes outside this range.
#define minAngleToWarrantTurningLeft           45
#define minAngleToWarrantTurningRight         -45
#define maxAngleToStopTurningLeft              35
#define maxAngleToStopTurningRight            -35

#define numberOfTrialsWhenChoosingBestPath    250 

// This is used for sending info back to the a computer.
#define sizeOfCircularBuffer                 2048

// Hardwire the number of tags (not counting the one on the robot).
#define numberOfAprilTagLocations               5

/** Global Variables **********************************************************/

// Make sure this is the robot running the code (the number is from the April Tag). 
uint8 XDATA myID          = (uint8) 26;     //       <--------------------------------------------------------------------------

uint8 XDATA currentMode;

// Added by JWS to allow explicit waits between actions.
uint32 XDATA lastTimeRobotSeen      = (uint32) 0;
uint32 XDATA overallStartTimeInMsec = (uint32) 0;
int16  XDATA state                  = ( int16)-1;

BIT   waitForKeyPressToStart   = (BIT)  1;

int8  XDATA tag                = (int8) 0; // Rather than passing this around a lot, store once the index of the AprilTag currently being targeted.
int8  XDATA numberOfTagsFound  = (int8) 0; // Found in the CAMERA.
int8  XDATA aprilTagsFound     = (int8) 0; // Found by driving the robot to them.


BIT         foundMyTag    = (BIT)   0;
float XDATA angleToTurn   = (float) 0.0;
float XDATA angleToApril  = (float) 0.0;
float XDATA distanceToTag = (float) 0.0;

char  XDATA str[64];
char  XDATA circularBuffer[sizeOfCircularBuffer];
int   XDATA circBuffReadIndex  = 0; // Keep a next READ location (ie, read bytes from buffer then send back to console).
int   XDATA circBuffWriteIndex = 0; // Keep a next WRITE location (the code does NOT check if WRITE overwrites something not yet sent back, so data can be lost of writing too much, too quickly - increase sizeOfCircularBuffer if you have the spare memory).

int8  XDATA firstTime = (int8) -1; // -1 means print initial string, 1 means it is the first time, and 0 means it isn't.

int8  XDATA currentAction = (int8) 0; // 0 = NONE, 1 = LEFT, 2 = RIGHT, 3 = FORWARD, 4 = BACKWARD (not used)

/** STRUCTURES ****************************************************************/

typedef XDATA struct AprilTagLocation {
  int16  x;     
  int16  y;     
  int16  angle; 
  uint16 count; // Used to count number of measurements.
  char   name;
  uint8  visited; // Cannot use BIT here.  Guess since already assigned to XDATA.
} AprilTagLocation;

AprilTagLocation        locations[numberOfAprilTagLocations];
AprilTagLocation backup_locations[numberOfAprilTagLocations];

AprilTagLocation myLocation;
uint8 listOfTagsFound[numberOfAprilTagLocations];

BIT readstate=0;


/** Parameters (that appeared in the provided code) ****************************************************************/
#define MODE_TETHERED	    0
#define MODE_UNTETHERED		1

#define A1	0
#define A2	1
#define B1	2
#define B2	3
#define ENABLE	15

int32 CODE param_baud_rate = (int32) 115200;  // <----------------------------------


/** Functions *****************************************************************/
void sendBackString(char s[]);
void goForward();
//void goBackward();
void turnLeft();
void turnRight();
void brake();
void stop();
void readCameraInfo();
void readPackets();

float keepAngleInRange(float XDATA angle);

void timer3Init();
void updateLeds();
void updateMode();
void usbToRadioService();

// Functions for paths.
float distance(int16 XDATA x1, int16 XDATA y1, int16 XDATA x2, int16 XDATA y2) {
	float XDATA  distanceSq = (float) (x1 - x2) * (float) (x1 - x2) + (float) (y1 - y2) * (float) (y1 - y2);
	return sqrtf(distanceSq);
}

float scorePath() {
	int16 XDATA lastX = myLocation.x;
	int16 XDATA lastY = myLocation.y;
	float XDATA score = 0.0;
	
	for (tag = (int8) 0; tag < (int8) numberOfAprilTagLocations; tag++) {
		if (locations[tag].visited == (uint8) 0) {
			score += distance(lastX, lastY, locations[tag].x, locations[tag].y);
			lastX = locations[tag].x;
			lastY = locations[tag].y;
		}
	} tag = (int8) 0; // Reset for (probably unneeded) safety.
	return score;
} 

void findNextUnvisitedLocation() { // Here we want to change the GLOBAL tag.
	for (tag = (int8) 0; tag < (int8) numberOfAprilTagLocations; tag++) {
		if (locations[tag].visited == (uint8) 0) { return; }
	}
	#ifdef shortStrings
	sendBackString("\n\rDONE!\n\r");
	#else
	sendBackString("\n\rSeems all tags have been visited.\n\r");
	#endif
	tag = -1;
}


// Cannot pass structures as arguments, so pass pointers.
void copyContents(AprilTagLocation* fromTag, AprilTagLocation* toTag) {
	toTag->x       = fromTag->x;
	toTag->y       = fromTag->y;
	toTag->visited = fromTag->visited;
	toTag->name    = fromTag->name;
	toTag->angle   = fromTag->angle;
	toTag->count   = fromTag->count;
}

void copyAprilTagsListFromBackup() {
	for (tag = (int8) 0; tag < (int8) numberOfAprilTagLocations; tag++) {
		copyContents(&backup_locations[tag], &locations[tag]);
	} tag = (int8) 0;
}

void copyAprilTagsListToBackup() {
	for (tag = (int8) 0; tag < (int8) numberOfAprilTagLocations; tag++) {
		copyContents(&locations[tag], &backup_locations[tag]);
	} tag = (int8) 0;
}

// Uniformly returns a number in [0, 1).
float randomInO1() {  // rand() returns a number in 0, ..., RAND_MAX.  The 0.999999, while a bit of a hack, prevents integer division and means the max result is n-1.
	return ((0.999999 * rand()) / RAND_MAX);
}

// Return random integer in 0, ..., n-1.
int getRandomInRangeExclusive(int XDATA n) {
	return (int) (randomInO1() * n);
}

// See http://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle
void permuteAprilTagsList() {
//	for i from 0 to n - 1 do
//       j <- random integer with i <= j < n
//       exchange a[j] and a[i]
   for (tag = 0; tag < numberOfAprilTagLocations; tag++) {
		AprilTagLocation temp;
	    int j = tag + getRandomInRangeExclusive(numberOfAprilTagLocations - tag);
		copyContents(&locations[j],   &temp);
		copyContents(&locations[tag], &locations[j]);
		copyContents(&temp,           &locations[tag]); // Can't swap structures, but maybe swap MEMORY LOCATIONS (ie, the & in C).  Or are the structures 'in lined' in C?
   } tag = (int8) 0;
}

void reportPath() {
	#ifdef shortStrings
	char XDATA reportPathString[] = "  ME %c @[%ix%i] ang = %i\n\r";
	char XDATA reportTagString[]  = "   T%c@[%ix%i]\n\r";
	#else
	char XDATA reportPathString[] = "  Robot %c [%i, %i] ang = %i\n\r";
	char XDATA reportTagString[]  = "     -> Tag %c [%i, %i]\n\r";
	#endif
	
	sprintf(str, reportPathString, myLocation.name, (int) myLocation.x, (int) myLocation.y, (int) myLocation.angle);
	sendBackString(str);
	
	for (tag = (int8) 0; tag < (int8) numberOfAprilTagLocations; tag++) {
		sprintf(str, reportTagString, locations[tag].name, (int) locations[tag].x, (int) locations[tag].y);
		sendBackString(str);
	} tag = (int8) 0;
}


float XDATA bestScore     = 0.0; // Length of best path found (will get intialized later so ok to use 0 here).
int16 XDATA randomSamples = 0;   // Count the number of random paths generated (we score one per 'cycle' until max reached).

// A crude method for finding the shortest path.  Try N random solutions put the best scoring one in locations.  Return best score.
void findLeastCostPath() {
	float XDATA score;
	
	if (randomSamples == 0) { // First time.
		copyAprilTagsListToBackup(); // Use backup for the best score so far.
		bestScore = scorePath(); 
		randomSamples++;  // Count the initial sample (plus need to increment or we'll revist this IF next time!).
		return;
	}
	
	if (randomSamples == numberOfTrialsWhenChoosingBestPath) { // See if done.
	    #ifdef shortStrings
		char  XDATA patternAtEnd[] = "Best = %i\n\r";
		#else
		char  XDATA patternAtEnd[] = "Least cost = %i.\n\r";
		#endif
		
		copyAprilTagsListFromBackup();
		randomSamples++; // Increment so we know we've done the final processing.
	    #ifndef shortStrings
		sendBackString("\n\rBest path found:\n\r");
		#endif
		reportPath();
		sprintf(str, patternAtEnd, (int) bestScore);
		sendBackString(str);
		tag = (int8) 0; // Set elsewhere, but might as well do here also.
		return;
	}
	
	permuteAprilTagsList();
	randomSamples++; 
	score = scorePath();   
	if (score < bestScore) {
		char XDATA reportCostString[] =  "\n\rNew least cost = %i on trial %i.\n\r";
		bestScore = score;
		copyAprilTagsListToBackup();
		#ifdef waitBetweenActions
		sprintf(str, reportCostString, (int) bestScore, j);
		sendBackString(str);
		reportPath();
		#endif
	}
}

void initializeForShiraTaskOne() {
	#ifdef shortStrings
	char  XDATA reportInitString[] = "Init Task 1 t=%i.\n\r";
	#else
	char  XDATA reportInitString[] = "Init Shira Task 1, #trials = %i.\n\r";
	#endif
	int8 i;
	
	sprintf(str, reportInitString, numberOfTrialsWhenChoosingBestPath);
	sendBackString(str);
		
	findLeastCostPath();
	firstTime = (int8) 0;
	
	for (i = (int8) 0; i < numberOfAprilTagLocations; i++) {
		listOfTagsFound[i] == (uint8) 101; // Mark that NOTHING in this array cell (dangerous to use 0 in case that is a robot ID).
	}
	
	#ifdef shortStrings
	sendBackString("Init done\n\r");
	#else
	sendBackString("InitializeForShiraTaskOne done.\n\r");
	#endif
}

float keepAngleInRange(float XDATA angle) {
	if (angle >  -180.0 && angle <= 180.0) return angle;
	if (angle <= -180.0)                   return keepAngleInRange(angle + 360.0);
    return                                        keepAngleInRange(angle - 360.0);
}

float maxf(float XDATA x1, float XDATA x2) {
	if (x1 >= x2) return x1;
	return x2;
}

float minf(float XDATA x1, float XDATA x2) {
	if (x1 <= x2) return x1;
	return x2;
}

float radiansToDegrees(float XDATA angleInRadians) {
	return (360.0 * angleInRadians) / TWO_PI;
}
float degreesToRadians(float XDATA angleInDegrees) {
	return (TWO_PI * angleInDegrees) / 360.0;
}

// void reportTag() {
// #ifdef reportInfo
	// char XDATA pattern[] = "Go to Tag %c @ %i x %i\n\r";
	// sprintf(str, pattern, locations[tag].name, (int) locations[tag].x, (int) locations[tag].y);
	// sendBackString(str);
// #endif
// }

void reportRobotLocation() {
	#ifdef shortStrings
	char XDATA report[] = "\n\rME@%ix%i %ideg ang to T%c %i\n\r";
	#else
	char XDATA report[] = "\n\rRobot@%ix%i, %i deg; ang to Tag %c = %i\n\r";
	#endif
	
	#ifdef reportInfo
	sprintf(str, report, (int) myLocation.x, (int) myLocation.y, (int) myLocation.angle, locations[tag].name, (int) angleToApril);
	sendBackString(str);
	#endif
}

void reportGoForward() {
	#ifdef shortStrings
	char XDATA report[] = "F T%c a=%i d=%i\n\r";
	#else
	char XDATA report[] = "Goto Tag %c, %i deg, dist = %i\n\r";
	#endif
	
	#ifdef reportInfo
	reportRobotLocation();
	sprintf(str, report, locations[tag].name, (int) angleToTurn, (int) distanceToTag);
	sendBackString(str);
	#endif
}

void reportTurn(float angle) {
	#ifdef shortStrings
	char XDATA report[] = "%c r=%i to T%c\n\r";
	#else
	char XDATA report[] = "Turn %c %i deg toward Tag %c\n\r";
	#endif
	
	#ifdef reportInfo
	reportRobotLocation();
	sprintf(str, report, (angle > 0 ? 'L' : 'R'), (int) angleToTurn, locations[tag].name);
	sendBackString(str);
	#endif
}

void setPower(uint8 XDATA power) {
	if (T3CC0 != power || T3CC1 != power) { T3CC0 = T3CC1 = power; }
}

float distanceFromRobotToTag() {
	return distance(locations[tag].x, locations[tag].y, myLocation.x, myLocation.y);
}

void moveForward() {
	if (distanceFromRobotToTag() < (float) 2.0 * (float) minDistanceToBeCloseEnoughToAprilTag) {
		setPower((uint8) powerForGoingForwardSlowly); // Slow up when close.   Now we do? BUT NOTE THAT WE DON'T CHANGE POWER IN THE MIDDLE OF A MOVE-FORWARD.
	} else {
		setPower((uint8) powerForGoingForward);
	}
	if (currentAction == (int8) 3) return; // Already going forward.
	
	reportGoForward();
	goForward();
}

void turnTowardTag() {
	if (angleToTurn > (float) -30.0 && angleToTurn < (float) 30.0) {
		setPower((uint8) powerForTurningSlowly); // Slow down for small angles.  Now we do? BUT NOTE THAT WE DON'T CHANGE POWER IN THE MIDDLE OF A TURN.
	} else {
		setPower((uint8) powerForTurning);
	}
	
	if (angleToTurn <  0 && currentAction == (int8) 2) return; // Already going right.
	if (angleToTurn >= 0 && currentAction == (int8) 1) return; // Already going left.
	
	reportTurn(angleToTurn);
	if (angleToTurn < 0) {
		turnRight();
	} else {
		turnLeft();
	}
}

void moveTowardAprilTag() {
	angleToApril = keepAngleInRange(radiansToDegrees(atan2f((float) (locations[tag].y - myLocation.y), 
															(float) (locations[tag].x - myLocation.x))));   // http://en.wikipedia.org/wiki/Atan2
	angleToTurn  = keepAngleInRange(angleToApril - (float) myLocation.angle);
		
	if        ((currentAction == (int8) 0) && (angleToTurn > (float) minAngleToWarrantTurningRight) && (angleToTurn < (float) minAngleToWarrantTurningLeft)) {
		moveForward(); // Was not moving.  Use full-width angles on both sides to decide if going forward is ok.
	} else if ((currentAction == (int8) 3) && (angleToTurn > (float) minAngleToWarrantTurningRight) && (angleToTurn < (float) minAngleToWarrantTurningLeft)) {
		moveForward(); // Already going forward, so no change needed, but might want to slow up. 
	} else if ((currentAction == (int8) 1) && (angleToTurn > (float) minAngleToWarrantTurningRight) && (angleToTurn < (float) maxAngleToStopTurningLeft))    {
		moveForward(); // Currently turning left.  Use a smaller angle, on the LEFT SIDE ONLY, to decide when to ok to go forward to reduce oscillations.
	} else if ((currentAction == (int8) 2) && (angleToTurn > (float) maxAngleToStopTurningRight)    && (angleToTurn < (float) minAngleToWarrantTurningLeft)) {
		moveForward(); // Currently turning right.  Use a smaller angle again, but this time on the RIGHT.
	} else {
		turnTowardTag();
	}
}

BIT closeEnoughToAprilTag() {
	distanceToTag = distanceFromRobotToTag();
	
	if (distanceToTag <= (float) minDistanceToBeCloseEnoughToAprilTag) {
		#ifdef shortStrings
		char  XDATA pattern[] = "\n\rGot T%c, dis=%i.\n\r";
		#else
		char  XDATA pattern[] = "\n\rFound it! Distance to Tag %c is %i.\n\r";
		#endif
		
		sprintf(str, pattern, locations[tag].name, (int) distanceToTag);
		sendBackString(str);
		return (BIT) 1;
	}
	return (BIT) 0;
}

int8 tagAlreadyFound(uint8 XDATA someTag) {
	int8  XDATA i;
	for (i = (int8) 0; i < numberOfAprilTagLocations; i++) {
		if (listOfTagsFound[i] == someTag) return i;
	}
	return (int8) -1;
}

int8 XDATA packetsRead = (int8) 1;
void readFourPackets() { // Do up to four MORE bytes.
	if (packetsRead >= (int8) 5) return;
	if (radioComRxAvailable()) { 
		packet.bytes[packetsRead] = radioComRxReceiveByte(); 
		packetsRead++; 
		//readFourPackets();
	}
}

void readCameraInfo() { // Only the first time do we care about the locations of the April Tags not on our robot, but due to noise, we repeatedly record the position (taking the average).
	uint8 XDATA ID;
	
	readFourPackets();
	if (packetsRead < (int8) 5) return; // Wait until all 5 bytes read.
	
	ID = readID(&packet);
	if (myID == ID) { // Need to continually update these (x, y, and angle) for the robot, since it is moving and rotating.
		myLocation.y      = (int16)          ((uint16) 480 - readY(&packet));
		myLocation.x      = (int16)                          readX(&packet);
		myLocation.angle  = (int16) keepAngleInRange((float) readR(&packet));

		if (myLocation.count < (uint16) 32000) myLocation.count++; // Watch for wraparound (can go to 65K since unsigned, but play it safe).
		else                                   myLocation.count = (uint16) 1;
		if (myLocation.count % 16 == 1) lastTimeRobotSeen = getMs(); // This might be slow, so only call every now and then. Mod 32 should be about 1 ssecond since frame rate is 30 camera sbhots per second.
		if (foundMyTag == (BIT) 0) {
			#ifdef shortStrings
			char  XDATA pattern[] = "ME T%c @ %ix%i ang=%i\n\r";
			#else
			char  XDATA pattern[] = "Found ME Tag %c @ %ix%i ang = %i.\n\r";
			#endif
			
			myLocation.name = (char) (myID + (uint8) offsetForName);
			sprintf(str, pattern, myLocation.name, (int) myLocation.x, (int) myLocation.y, (int) myLocation.angle);
			sendBackString(str);
			foundMyTag = (BIT) 1;  // This only matters the first time around, when we are waiting until ALL tags located.
		}
	} else { // NOW DO A FEW MORE, TO BE ROBUST TO BAD SENSING: if (numberOfTagsFound < (int8) numberOfAprilTagLocations) { // Keep reading until the expected number of April Tags have been read.
		int8 tagIndex  = (int8) numberOfTagsFound;
		int8 tagExists = tagAlreadyFound(ID); // See if this tag is already in the i'th cell of the locations array.
		float XDATA alpha;	
		
		if (tagExists >= (int8) 0) { // This object is already an item in the list.
			tagIndex = tagExists;
		} else if (numberOfTagsFound >= numberOfAprilTagLocations) { // Have already seen enough robots, so this must be a fluke (or an earlier one was ...).
			return;
		// } else {
			// char XDATA pattern2[] = "FIRST TIME: ID %i T %i ex=%i\n\r";
			// sprintf(str, pattern2, (int) ID, (int) numberOfTagsFound, (int) tagExists);
			// sendBackString(str);
		}
		
		if (locations[tagIndex].count < (uint16) 32000) locations[tagIndex].count++; // Watch for wraparound.
		if (locations[tagIndex].count % 32 == (uint16) 1 ) && locations[tagIndex].count < (uint16) 3200) { // Update 100 times in case there is noise (and do once every 32 cycles, which is about once per second).
			alpha = (float) 1.0 / (float) locations[tagIndex].count;
			locations[tagIndex].y     = (int16)                 ((float) locations[tagIndex].y     * ((float) 1.0 - alpha) +   (float) ((uint16) 480 - readY(&packet)) * alpha); // Do in-place averaging.
			locations[tagIndex].x     = (int16)                 ((float) locations[tagIndex].x     * ((float) 1.0 - alpha) +                   (float) readX(&packet)  * alpha); // Do we have enough bits?
			locations[tagIndex].angle = (int16) keepAngleInRange((float) locations[tagIndex].angle * ((float) 1.0 - alpha) +  keepAngleInRange((float) readR(&packet)) * alpha);
		}
		// Be sure the x and y updated at least once before doing this.
		if (tagExists < (int8) 0) {
			#ifdef shortStrings
			char  XDATA pattern[] = "See T%c @ %ix%i\n\r";
			#else
			char  XDATA pattern[] = "Found Tag %c @ %ix%i.\n\r";
			#endif
			
			locations[tagIndex].name  = (char) (ID + (uint8) offsetForName);  // Convert to a letter so we can handle 26 tags.
			listOfTagsFound[tagIndex] = ID;
			sprintf(str, pattern, locations[tagIndex].name, (int) locations[tagIndex].x, (int) locations[tagIndex].y);
			sendBackString(str);
			numberOfTagsFound++;
		}
	} 
}

BIT checkAction() {	
	findNextUnvisitedLocation(); // This will properly set 'tag' (which might be also used in a preprocessing FOR loop).
	
	if (tag < (int8) 0) { // Done!  Ie, findNextUnvisitedLocation set the tag to a negative number.
		sendBackString("\n\r");
		sendBackString("X");
		sendBackString("\n\rAll tags found, so quit!\n\r");
		stop();
		return (BIT) 0;
	} else if (closeEnoughToAprilTag() != (BIT) 0) {  // At the current target?
		#ifdef shortStrings
		char XDATA actionPattern[] = "\n\r* HIT T%c (# %i) %s *\n\r";
		#else
		char XDATA actionPattern[] = "\n\r*** Reached Tag %c (# %i) at %i sec. ***\n\r";
		#endif
	//	brake(); // Maybe simply allow to keep driving and target the next object?
		currentAction = (int8) 0; // This will allow robot to change from going slow when near current target to full speed again if still going straight.
		aprilTagsFound++; // This variable is only used for reporting.
		sprintf(str, actionPattern, locations[tag].name, (int) aprilTagsFound, ((int)((getMs() - overallStartTimeInMsec))) / (int) 1000);
		sendBackString(str);
		locations[tag].visited = (uint8) 1;
	//	LED_RED((int)aprilTagsFound % 2 == 1);
		return checkAction();  // Maybe do NOT check again and instead simply wait until the next round?
	} else {
		moveTowardAprilTag();
	}
	return (BIT) 1;
}


void turnLeft() {
	//	sendBackString("Turn LEFT\n\r");
	setDigitalOutput(0,LOW); //Input A1 to motor driver, controls left side
	setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
	setDigitalOutput(2,LOW); //Input B1 to motor driver, controls right side
    setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
	currentAction = (int8) 1;
}

void turnRight() {
	//	sendBackString("Turn RIGHT\n\r");
    setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
    setDigitalOutput(1,LOW); //Input A2 to motor driver, controls left side
    setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
    setDigitalOutput(3,LOW); //Input B2 to motor driver, controls right side
	currentAction = (int8) 2;
}

void goForward() {
	//	sendBackString("Go FORWARD\n\r");
    setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
    setDigitalOutput(1,LOW); //Input A2 to motor driver, controls left side
    setDigitalOutput(2,LOW); //Input B1 to motor driver, controls right side
    setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
	currentAction = (int8) 3;
}

// Commented out to save memory.
// void goBackward() {
	//#ifdef waitBetweenActions
	//	endBackString("Go BACKWARD\n\r");
	//#endif
	// setDigitalOutput(0,LOW); //Input A1 to motor driver, controls left side
    // setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
    // setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
    // setDigitalOutput(3,LOW); //Input B2 to motor driver, controls right side
	// currentAction = (int8) 4;
// }

void brake() { // Hard brake.
	// sendBackString("BRAKE\n\r");
    setDigitalOutput(0,HIGH); //Input A1 to motor driver, controls left side
    setDigitalOutput(1,HIGH); //Input A2 to motor driver, controls left side
    setDigitalOutput(2,HIGH); //Input B1 to motor driver, controls right side
    setDigitalOutput(3,HIGH); //Input B2 to motor driver, controls right side
}

// This buffer is used to send characters back to the user's computer.
void initCircularBuffer() {
	for (circBuffReadIndex = 0; circBuffReadIndex < sizeOfCircularBuffer; circBuffReadIndex++) {
		circularBuffer[circBuffReadIndex] = '\0';
	}
	circBuffReadIndex  = 0;
	circBuffWriteIndex = 0;
}

// Might need to change this setting if communication not working well.
#define bytesToTransmitPerCall 16
void transmitFromCircularBuffer() {
    #ifdef transmitBack
	if (currentMode == MODE_UNTETHERED) {
		int XDATA counter = bytesToTransmitPerCall;
		while (counter > 0) {
			if (circularBuffer[circBuffReadIndex] != '\0') { 
				// if (!radioComTxAvailable()) { radioComTxService(); } // radioLinkTxQueueReset(); }
				if (radioComTxAvailable()) { // If service wasn't available, we'll simply send a few less char's this time.
					radioComTxSendByte(circularBuffer[circBuffReadIndex]);
					circularBuffer[circBuffReadIndex] = '\0'; // Mark that this was written out.
					circBuffReadIndex = (circBuffReadIndex + 1) % sizeOfCircularBuffer;
				}
				if (counter % 4 == 0) radioComTxService(); // Do this every now and then.
				counter--;
			} else {
				counter = 0;
			}
		}
	}
	#endif
}

void sendBackString(char s[]) {
	int XDATA i = 0;
	
	while (s[i] != '\0') {
		circularBuffer[circBuffWriteIndex] = s[i];
		circBuffWriteIndex = (circBuffWriteIndex + 1) % sizeOfCircularBuffer;
		i++;
	}
	circularBuffer[circBuffWriteIndex] = '\0'; // Mark end of string.
	transmitFromCircularBuffer(); // Transmit the first part of this immediately.  More will be sent each cycle through the main loop.
}


void mainPrep() {
    systemInit();
    usbInit();
    radioComRxEnforceOrdering = 0;
    radioComInit();
    setDigitalOutput(A1,    PULLED);
    setDigitalOutput(A2,    PULLED);
    setDigitalOutput(B1,    PULLED);
    setDigitalOutput(B2,    PULLED);
    setDigitalOutput(ENABLE,PULLED);
    timer3Init(); //Timer 3 will now control the Enable A and Enable B pins on the motor driver
    setDigitalOutput(A1, LOW); // Initializing A1, A2, B1, B2 to LOW so the robot doesn't move, but not brake mode
    setDigitalOutput(A2, LOW);
    setDigitalOutput(B1, LOW);
    setDigitalOutput(B2, LOW);
    setDigitalOutput(ENABLE, HIGH); //Standby mode: Motor driver turns off when LOW, on when HIGH
    T3CC0 = T3CC1 = 255; //this means we init PWM val to 255 unless otherwise specified

	overallStartTimeInMsec  = getMs();
	state                   =  (int16) -1000; // Indicates Shira's FIRST challenge task.
	numberOfTagsFound       =   (int8)     0;
	foundMyTag              =    (BIT)     0;
	
	setPower((uint8) 128);
}

void waitForG(uint8 XDATA byte) {
	if (byte == 0x67) {
		sendBackString("\n\rKey 'g' hit, so robot starting.\n\n\r");
		waitForKeyPressToStart = (BIT) 0;
		LED_RED((BIT)1);
	} 
}

void stop() {
	brake();
	radioComTxService();
	transmitFromCircularBuffer(); 
	sendBackString("\n\rExiting!\n\r"); // For some reason, this doesn't fully print, even with all the stuff below.  Guess not enough time.  But not worth adding busy-wait for just this.
	for (tag = (int8) 0; tag < (int8) 256; tag++) { // OK to reset 'tag' here since will be exiting shortly.
		radioComTxService();
		transmitFromCircularBuffer(); // Since exiting, attempt to clear buffer.
	}
}

BIT jwsRunRobot() {

	if (firstTime < (int8) 0) { // Wait for the user to press 'g' before doing anything.
		sendBackString("Press 'g' to start.\n\r");
		firstTime = (int8) 1;
	}
	
	if (readstate == (BIT) 1) { // Something from the camera needs to be read.
		readCameraInfo();
		if (packetsRead >= (int8) 5) { // See if done reading packets.  If not, loop around again.
			//LED_RED(0);
			packetsRead =  (int8) 0;
			readstate   =  (BIT)  0;
			if (numberOfTagsFound < (int8) numberOfAprilTagLocations || foundMyTag == (BIT) 0) { // See if all the tag locations have been read.
				// If not, continue the WHILE loop.
				return (BIT) 1; // Might as well return since nothing happens until all AprilTags located.
			}	 
			else if (firstTime > (int8) 0 && state > (int16)-2000 && state <= (int16)-1000) {
				initializeForShiraTaskOne(); // If first time all have been read, then chose a path to follow.
				return (BIT) 1; // Return here since initialization will have spent some cycles.
			}
		}
	} 
	
	else if (readstate == (BIT) 0 && radioComRxAvailable()) { // See if there is a byte waiting to be processed.  We want to clear these as soon as we can, to reduce the chances of overflows.
		uint8 byte = radioComRxReceiveByte();
	
		if (checkHeader(byte)) { // See if the 6 bytes of 1's are available.  Will set readstate=1.
			//LED_RED(1);
			packetsRead = (int8) 1;
			readstate = (BIT) 1;
		//	return (BIT) 1; // Seems ok to both process the camera and choose an action in one cycle.  Otherwise might continue to turn/move while locations of the non-moving April Tags are being read.
		}

		// Seems now ok to un-comment these?
		else if (waitForKeyPressToStart) {     // Wait for 'g' to be pressed to start.
			waitForG(byte);
			return (BIT) 1;
		}
			
		// else if (byte == 0x20) {               // Watch for the SPACE BAR.
			// sendBackString("\n\rSPACE BAR hit, stop!\n\r");
			// brake();
			// stop();
			// return (BIT) 0;
		// }
	}
	
	if (waitForKeyPressToStart) return (BIT) 1; // If waiting, simply return.
	
	if (state > (int16) -2000 && state <= (int16) -1000) { // Run the first game of Shira's.
		// See if sampling has completed.
		if (randomSamples <= numberOfTrialsWhenChoosingBestPath) { // Do one sample each cycle so all the other bookkeeping is properly done.
			if (firstTime == (int8) 0) findLeastCostPath(); // Might not have all the locations yet.
			return (BIT) 1;
		}
		
		// Move this outside the IF?
		if (currentAction != (int8) 0 && ((uint32) (getMs() - lastTimeRobotSeen)) > (uint32) 2500) {
			#ifdef shortStrings
			char  XDATA pattern[] = "No robot! t=%i, old=%i, diff=%i\n\r";
			#else
			char  XDATA pattern[] = "Did not see the robot for awhile, t = %i, last = %i, diff = %i.\n\r";
			#endif
			
			sprintf(str, pattern, (int) getMs(), (int) lastTimeRobotSeen, (int) (getMs() - lastTimeRobotSeen));
			sendBackString(str);
			currentAction = (int8) 0;
			brake();
			return (BIT) 1;
		}
		
		// Don't act unless robot seen recently.
		if (((uint32) (getMs() - lastTimeRobotSeen)) < (uint32) 2500) {
			return checkAction();
		}
	}
	
	return (BIT) 1;
}


void main()
{
	mainPrep();
    while (1)
    {
        updateMode();
        boardService();
        updateLeds();  
		radioComTxService();
        usbComService();
		transmitFromCircularBuffer(); // Don't put AFTER the call below to radioComTxService() since that seemed to mess up detecting the press of 'g'

        switch(currentMode) {
        case MODE_TETHERED:  	usbToRadioService();  break;
     // case MODE_UNTETHERED:	robotRadioService();  break;
		case MODE_UNTETHERED: 	
			if (!jwsRunRobot()) return;      
			break; 
        }
    }
}

/** Functions provided by Shira follow *****************************************************************/

void timer3Init()
{
    // Start the timer in free-running mode and set the prescaler.
    T3CTL = 0b01110000;   // Prescaler 1:8, frequency = (24000 kHz)/8/256 = 11.7 kHz
    //T3CTL = 0b01010000; // Use this line instead if you want 23.4 kHz (1:4)

    // Set the duty cycles to zero.
    T3CC0 = T3CC1 = 0;

    // Enable PWM on both channels.  We choose the mode where the channel
    // goes high when the timer is at 0 and goes low when the timer value
    // is equal to T3CCn.
    T3CCTL0 = T3CCTL1 = 0b00100100;

    // Configure Timer 3 to use Alternative 1 location, which is the default.
    PERCFG &= ~(1<<5);  // PERCFG.T3CFG = 0;

    // Configure P1_3 and P1_4 to be controlled by a peripheral function (Timer 3)
    // instead of being general purpose I/O.
    P1SEL |= (1<<3) | (1<<4);

    // After calling this function, you can set the duty cycles by simply writing
    // to T3CC0 and T3CC1.  A value of 255 results in a 100% duty cycle, and a
    // value of N < 255 results in a duty cycle of N/256.
}

void updateLeds()
{
    static BIT dimYellowLed = 0;
    static uint16 lastRadioActivityTime;
    uint16 now;

    usbShowStatusWithGreenLed();

    now = (uint16)getMs();

   if (!radioLinkConnected())
    {
        // We have not connected to another device wirelessly yet, so do a
        // 50% blink with a period of 1024 ms.
        LED_YELLOW(now & 0x200 ? 1 : 0);
    }
    else
    {
        // We have connected.

        if ((now & 0x3FF) <= 20)
        {
            // Do a heartbeat every 1024ms for 21ms.
            LED_YELLOW(1);
        }
        else if (dimYellowLed)
        {
            static uint8 DATA count;
            count++;
            LED_YELLOW((count & 0x7)==0);
        }
        else
        {
            LED_YELLOW(0);
        }
    }

    if (radioLinkActivityOccurred)
    {
        radioLinkActivityOccurred = 0;
        dimYellowLed ^= 1;
        //dimYellowLed = 1;
        lastRadioActivityTime = now;
    }

    if ((uint16)(now - lastRadioActivityTime) > 32)
    {
        dimYellowLed = 0;
    }
}


void updateMode()
{
    if (usbPowerPresent())
    {
	currentMode = MODE_TETHERED;       
    }
    else
    {
        currentMode = MODE_UNTETHERED; 
    }
}

void usbToRadioService() //runs during TETHERED mode, relays info between USB and radio
{
    // Data
    while(usbComRxAvailable() && radioComTxAvailable())
    {
        radioComTxSendByte(usbComRxReceiveByte());
    }

    while(radioComRxAvailable() && usbComTxAvailable())
    {
        usbComTxSendByte(radioComRxReceiveByte());
    }
}
