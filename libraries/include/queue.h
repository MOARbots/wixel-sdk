#ifndef MOAROBOT_QUEUE_H
#define MOAROBOT_QUEUE_H
/** A queue implemntation for Wixel. 
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */

#include<stdio.h>

//
// Remove the line "#define WIXEL"
// if you want to test code in standard c environment.
// The following macros help debugging in standard c environment.
// TODO: custimize a cross-platform Makefile to pass these flags
//
#define WIXEL

#ifdef WIXEL
  #include<wixel.h>
  #define MY_PRINT(str) 
  #define MY_PRINT2(str, arg) 
#else
  #define XDATA
  #define MY_PRINT(str) printf(str);
  #define MY_PRINT2(str, arg) printf(str, arg);
#endif

/*************************************
 * Container: Queue
 *************************************/
#define ContSize 64
#define TupleSize 5 

// forward decl.
struct Queue;

//
// FiveBytes Structure
//
typedef struct Tuple
{
  struct Queue* caller; 
  char bytes[TupleSize];
} Tuple;

// 
// Container: Queue 
//  - contains several fivebytes data
//  - it's circular queue implemenation
// 
typedef struct Queue
{
  // member data
  Tuple data[ContSize]; // we can only use ContSize-1 slots
  int head; // first elelment position.
  int tail; // last element position.

  // member function declariation
  int (*Push)(Tuple* p);
  int (*Pop)(Tuple* p);
  int (*PushForce)(Tuple* p);
} Queue;


int InitQueue(Queue* queue);
int LinkQueueTuple(Queue* queue, Tuple* tuple); 
int Push(Tuple* p);
int PushForce(Tuple* p);
int Pop(Tuple* p);

#endif
