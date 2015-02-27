#include "queue.h"
//
// member funciton: Push 
// return 1: success, 
//        0: fail(queue is full)
//
int Push(Tuple* p) 
{
  int i = 0;
  int next = 0;
  Queue* queue = (0==p ? 0: p->caller);
  if (0!=queue) {
    next = (queue->tail+1)%ContSize;
    if (next != queue->head) {
      // copy p to queue
      for (i = 0; i < TupleSize; ++i) {
        queue->data[next].bytes[i] = p->bytes[i];
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
int PushForce(Tuple* p) 
{
  int i = 0;
  int next = 0;
  Queue* queue = (0==p ? 0: p->caller);
  if (0!=queue) {
    next = (queue->tail+1)%ContSize;
    if (next != queue->head) {
      // copy p to queue
      for (i = 0; i < TupleSize; ++i) {
        queue->data[next].bytes[i] = p->bytes[i];
      } 
      queue->tail = next;
      return 1;
    } else {
      for (i = 0; i < TupleSize; ++i) {
        queue->data[next].bytes[i] = p->bytes[i];
      } 
      queue->tail = next;
      queue->head = (queue->head+1)%ContSize;;
      return 1;
    }
  }
  return 0;
}



//
// member funciton: Pop
// return 1: success, 
//        0: fail(queue is already empty)
//
int Pop(Tuple* p) 
{
  int i = 0;
  Queue* queue = 0==p ? 0 : p->caller;
  if (0!=queue && queue->head!=queue->tail) {
    queue->head = (queue->head+1)%ContSize;
    // copy queue to p
    for (i = 0; i < TupleSize; ++i) {
      p->bytes[i] = queue->data[queue->head].bytes[i];
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
    queue->Push = Push;
    queue->PushForce = PushForce;
    queue->Pop = Pop;
    return 1;
  }
  return 0;
}




//
// Associate Queue and Tuple. 
// return 1: success
//        0: fail
//
int LinkQueueTuple(Queue* queue, Tuple* tuple) 
{
  if (0!=queue && 0!=tuple) {
    tuple->caller = queue;
    return 1;
  }
  return 0;
}



