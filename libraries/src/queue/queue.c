#include "queue.h"

/** A queue implemntation for Wixel. 
 *
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */


//
// member funciton: Push 
// return 1: success, 
//        0: fail(queue is full)
//
int QPush(QTuple* p) __reentrant
{
  int i = 0;
  int next = 0;
  Queue* queue = (0==p ? 0: p->caller);
  if (0!=queue) {
    next = (queue->tail+1)%QContSize;
    if (next != queue->head) {
      // copy p to queue
      for (i = 0; i < QTupleSize; ++i) {
        queue->Qdata[next].bytes[i] = p->bytes[i];
      } 
      queue->tail = next;
      return 1;
    }
  }
  return 0;
}

//
// Force push date into the queue.
// i.e. if the queue is full, then the new pushed elelment will overide oldest element.
//
int QPushForce(QTuple* p) __reentrant
{
  int i = 0;
  int next = 0;
  Queue* queue = (0==p ? 0: p->caller);
  if (0!=queue) {
    next = (queue->tail+1)%QContSize;
    if (next != queue->head) {
      // copy p to queue
      for (i = 0; i < QTupleSize; ++i) {
        queue->Qdata[next].bytes[i] = p->bytes[i];
      } 
      queue->tail = next;
      return 1;
    } else {
      for (i = 0; i < QTupleSize; ++i) {
        queue->Qdata[next].bytes[i] = p->bytes[i];
      } 
      queue->tail = next;
      queue->head = (queue->head+1)%QContSize;;
      return 1;
    }
  }
  return 0;
}



//
// member function: Pop
// return 1: success, 
//        0: fail(queue is already empty)
//
int QPop(QTuple* p) __reentrant
{
  int i = 0;
  Queue* queue = 0==p ? 0 : p->caller;
  if (0!=queue && queue->head!=queue->tail) {
    queue->head = (queue->head+1)%QContSize;
    // copy queue to p
    for (i = 0; i < QTupleSize; ++i) {
      p->bytes[i] = queue->Qdata[queue->head].bytes[i];
    }
    return 1;
  }
  return 0;
}

//
// Call this before using Queue
// return 1: success
//        0: fail
//
int InitQueue(Queue* queue) 
{
  if (0!=queue) {
    queue->head = 0;
    queue->tail = 0;
    queue->Push = QPush;
    queue->PushForce = QPushForce;
    queue->Pop = QPop;
    return 1;
  }
  return 0;
}




//
// Associate Queue and QTuple. 
// return 1: success
//        0: fail
//
int LinkQueue(Queue* queue, QTuple* tuple) 
{
  if (0!=queue && 0!=tuple) {
    tuple->caller = queue;
    return 1;
  }
  return 0;
}



