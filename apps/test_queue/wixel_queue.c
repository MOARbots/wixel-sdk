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
} Queue;

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


//
// Test push and pop
//
int TestFunction1()
{
  // the queue is a container to store Tuple data
  // you can redefine the TupleSize to change the max size that a Tuple can store. 
  // here, for example the size is 5.
  // Note: the "XDATA" indicate that Queue's data is in external ram, 
  //       which is larger, but slower. If you remove XDATA, then make ContSize smaller.
  // ref: http://pololu.github.io/wixel-sdk/cc2511__types_8h.html
  XDATA Queue queue;
  // we will push two fivebytes to queue
  Tuple fivebytes;
  // then pop them out to pop_result 
  Tuple pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and Tuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue);
  LinkQueueTuple(&queue, &fivebytes);
  LinkQueueTuple(&queue, &pop_result);

  for (i = 0; i < ContSize; i++) {
    // test push "hello" which is five byte
    fivebytes.bytes[0] = 'h';
    fivebytes.bytes[1] = 'e';
    fivebytes.bytes[2] = 'l';
    fivebytes.bytes[3] = 'l';
    fivebytes.bytes[4] = 'o';
    if (queue.Push(&fivebytes)) {
      MY_PRINT(" push \n");
    } else {
      MY_PRINT(" full \n");
    }

    // test push "world" which is five byte
    fivebytes.bytes[0] = 'w';
    fivebytes.bytes[1] = 'o';
    fivebytes.bytes[2] = 'r';
    fivebytes.bytes[3] = 'l';
    fivebytes.bytes[4] = 'd';
    if (queue.Push(&fivebytes)) {
      MY_PRINT(" push \n");
    } else {
      MY_PRINT(" full \n");
    }
  }

  // test pop out all data
  while(queue.Pop(&pop_result)) {
    for (i = 0; i < TupleSize; ++i) {
     MY_PRINT2("%c", pop_result.bytes[i]);
    }
    MY_PRINT("\n");
  }
  MY_PRINT("\n");
  return 0;
}


//
// Test multiple Queues
//
int TestFunction2()
{
  // the queue is a container to store Tuple data
  // you can redefine the TupleSize to change the max size that a Tuple can store. 
  // here, for example the size is 5.
  XDATA Queue queue1;
  XDATA Queue queue2;
  // we will push two fivebytes to queue
  Tuple fivebytes1, fivebytes2;
  // then pop them out to pop_result 
  Tuple pop_result1, pop_result2;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and Tuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue1);
  LinkQueueTuple(&queue1, &fivebytes1);
  LinkQueueTuple(&queue1, &pop_result1);

  InitQueue(&queue2);
  LinkQueueTuple(&queue2, &fivebytes2);
  LinkQueueTuple(&queue2, &pop_result2);

  // test: push "hello" which is five byte
  fivebytes1.bytes[0] = 'h';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 'l';
  fivebytes1.bytes[3] = 'l';
  fivebytes1.bytes[4] = 'o';
  queue1.Push(&fivebytes1);

  // test: push "world" which is five byte
  fivebytes2.bytes[0] = 'w';
  fivebytes2.bytes[1] = 'o';
  fivebytes2.bytes[2] = 'r';
  fivebytes2.bytes[3] = 'l';
  fivebytes2.bytes[4] = 'd';
  queue2.Push(&fivebytes2);

  // test: queue1 pop out all data
  while(queue1.Pop(&pop_result1)) {
    for (i = 0; i < TupleSize; ++i) {
      MY_PRINT2("%c", pop_result1.bytes[i]);
    }
    MY_PRINT("\n");
  }

  // test: queue2 pop out all data
  while(queue2.Pop(&pop_result2)) {
    for (i = 0; i < TupleSize; ++i) {
     MY_PRINT2("%c", pop_result2.bytes[i]);
    }
    MY_PRINT("\n");
  }
  return 0;
}

int main()
{
  // The test function shows how to use Queue

  // Test one local queue
  MY_PRINT("Test1:\n");
  TestFunction1();
  // Test two local queues
  MY_PRINT("Test2:\n");
  TestFunction2();
  return 0;
}


