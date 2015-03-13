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

/** 
 * Container: Queue 
 *  - contains several fivebytes data
 *  - it's circular queue implemenation
 */ 
#define USE_QUEUE_TUPLE_TYPE(queue_name, queue_size, tuple_name, tuple_size) \
\
typedef struct tuple_name \
{\
  char  magic;\
  char  size;\
  void* caller;\
  char  bytes[tuple_size];\
}tuple_name ;\
\
typedef struct queue_name \
{\
  char  magic;\
  char  size;\
  int head;\
  int tail;\
  int (*Push)(     void* p) __reentrant; \
  int (*PushForce)(void* p) __reentrant; \
  int (*Pop)(      void* p) __reentrant; \
  tuple_name Qdata[queue_size];\
} queue_name;


/** Init and Associate Queue with QTuple. 
 *  return 1: success
 *         0: fail
 *  note: size must <= queue_size in the macro above.
 *        size must <= tuple_size in the macro above.
 */
int InitQueue(void* queue, int size); 
int LinkQueue(void* queue, void* tuple, int tuple_size); 

#endif
