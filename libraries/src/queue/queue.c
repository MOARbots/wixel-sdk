#include "queue.h"

/** A queue implemntation for Wixel. 
 *
 *  Author: Lun-Cheng Chu
 *   Mail: lc@cs.wisc.edu
 */

/** runtime type check
 */
#define QUEUE_MAGIC 20
#define TUPLE_MAGIC 16

/** handy dummy type
 */
USE_QUEUE_TUPLE_TYPE(DummyQueue, 1, DummyTuple, 1)

//
// member funciton: Push 
// return 1: success, 
//        0: fail(queue is full)
//
int QPush(void* v) __reentrant
{
  int i = 0;
  int next = 0;
  int q_size = 0;
  int t_size = 0;
  DummyTuple* tuple = (DummyTuple*)v;
  DummyQueue* queue = (DummyQueue*)(0==tuple ? 0: tuple->caller);

  // sanity check
  if (0==tuple || tuple->magic!=TUPLE_MAGIC)
    return 0;
  if (0==queue || queue->magic!=QUEUE_MAGIC)
    return 0;

  q_size = queue->size;
  t_size = tuple->size;

  next = (queue->tail+1)%q_size;
  if (next != queue->head) {
    // copy tuple to queue
    for (i = 0; i < t_size; ++i) {
      queue->Qdata[next].bytes[i] = tuple->bytes[i];
    } 
    queue->tail = next;
    return 1;
  }
  return 0;
}

//
// Force push date into the queue.
// i.e. if the queue is full, then the new pushed elelment will overide oldest element.
//
int QPushForce(void* v) __reentrant
{
  int i = 0;
  int next = 0;
  int q_size = 0;
  int t_size = 0;

  DummyTuple* tuple = (DummyTuple*)v;
  DummyQueue* queue = (DummyQueue*)(0==tuple ? 0: tuple->caller);

  if (0!=queue) {
    q_size = queue->size;
    t_size = tuple->size;

    next = (queue->tail+1)%q_size;
    if (next != queue->head) {
      // copy tuple to queue
      for (i = 0; i < t_size; ++i) {
        queue->Qdata[next].bytes[i] = tuple->bytes[i];
      } 
      queue->tail = next;
      return 1;
    } else {
      for (i = 0; i < t_size; ++i) {
        queue->Qdata[next].bytes[i] = tuple->bytes[i];
      } 
      queue->tail = next;
      queue->head = (queue->head+1)%q_size;;
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
int QPop(void* v) __reentrant
{
  int i = 0;
  int q_size = 0;
  int t_size = 0;
  DummyTuple* tuple = (DummyTuple*)v;
  DummyQueue* queue = (DummyQueue*)(0==tuple ? 0: tuple->caller);

  if (0!=queue && queue->head!=queue->tail) {
    q_size = queue->size;
    t_size = tuple->size;

    queue->head = (queue->head+1)%q_size;
    // copy queue to p
    for (i = 0; i < t_size; ++i) {
      tuple->bytes[i] = queue->Qdata[queue->head].bytes[i];
    }
    return 1;
  }
  return 0;
}

/** Associate Queue and QTuple. 
 *  return 1: success
 *         0: fail
 */

int InitQueue(void* queue, int size) 
{
  DummyQueue* q = (DummyQueue*)queue;

  if (0!=q) {
    q->magic = QUEUE_MAGIC;
    q->size = size;
    q->head = 0;
    q->tail = 0;
    q->Push = QPush;
    q->PushForce = QPushForce;
    q->Pop = QPop;
    return 1;
  }
  return 0;
}

int LinkQueue(void* queue, void* tuple, int size)
{
  DummyQueue* q = (DummyQueue*)queue;
  DummyTuple* t = (DummyTuple*)tuple;

  if (0!=q && 0!=t) {
    t->magic = TUPLE_MAGIC;
    t->size = size;
    t->caller = queue;
    return 1;
  }
  return 0;
}



