#ifndef MOAROBOT_QUEUE_H
#define MOAROBOT_QUEUE_H

/** A queue implemntation for Wixel. 
 *
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */

#include "platform_util.h"

/*************************************
 * Container: Queue
 *************************************/
#define QContSize 64
#define QTupleSize 5 

// forward decl.
struct Queue;

//
// FiveBytes Structure
//
typedef struct QTuple
{
  struct Queue* caller; 
  char bytes[QTupleSize];
} QTuple;

// 
// Container: Queue 
//  - contains several fivebytes data
//  - it's circular queue implemenation
// 
typedef struct Queue
{
  // member data
  QTuple Qdata[QContSize]; // we can only use ContSize-1 slots
  int head; // first elelment position.
  int tail; // last element position.

  // member function declariation
  int (*Push)(QTuple* p);
  int (*Pop)(QTuple* p);
  int (*PushForce)(QTuple* p);
} Queue;


int InitQueue(Queue* queue);
int LinkQueue(Queue* queue, QTuple* tuple); 

#endif
