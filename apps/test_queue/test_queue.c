#include "queue.h"

/** Define type Queue1 with Tuple1 size 5
 *  and Queue1 can store at most 4 tuples
 */
#define QUEUE1_SIZE 4 
#define TUPLE1_SIZE 5
USE_QUEUE_TUPLE_TYPE(Queue1, QUEUE1_SIZE, Tuple1, TUPLE1_SIZE)

/** Define type Queue2 with Tuple1 size 3
 *  and Queue2 can store at most 12 tuples
 */
#define QUEUE2_SIZE 12
#define TUPLE2_SIZE  3
USE_QUEUE_TUPLE_TYPE(Queue2, QUEUE2_SIZE, Tuple2, TUPLE2_SIZE)


//
// Test push and pop
//
int TestFunction1()
{
  // the queue is a container to store Tuple data
  // you can redefine the QTupleSize to change the max size that a Tuple can store. 
  // here, for example the size is 5.
  // Note: the "XDATA" indicate that Queue's data is in external ram, 
  //       which is larger, but slower. If you remove XDATA, then make ContSize smaller.
  // ref: http://pololu.github.io/wixel-sdk/cc2511__types_8h.html
  XDATA Queue1 queue;
  // we will push two fivebytes to queue
  Tuple1 fivebytes;
  // then pop them out to pop_result 
  Tuple1 pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  if (!InitQueue(&queue, QUEUE1_SIZE))
    return 0;
  if (!LinkQueue(&queue, &fivebytes, TUPLE1_SIZE))
    return 0;
  if (!LinkQueue(&queue, &pop_result, TUPLE1_SIZE))
    return 0;

  for (i = 0; i < queue.size; i++) {
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
    for (i = 0; i < pop_result.size; ++i) {
     MY_PRINT2("%c", pop_result.bytes[i]);
    }
    MY_PRINT("\n");
  }
  MY_PRINT("\n");
  return 0;
}


//
// Test multiple Queues with differnt tuple size
//
int TestFunction2()
{
  // the queue is a container to store Tuple data
  // here, for example the size is 5 and 3.
  XDATA Queue1 queue1; // tuple size is 5
  XDATA Queue2 queue2; // tuple size is 3
  // 5 bytes tuple
  Tuple1 fivebytes;
  // 3 bytes tuple
  Tuple2 threebytes;
  // then pop them out to pop_result 
  Tuple1 pop_result1;
  Tuple2 pop_result2;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue1, QUEUE1_SIZE);
  LinkQueue(&queue1, &fivebytes, TUPLE1_SIZE);
  LinkQueue(&queue1, &pop_result1, TUPLE1_SIZE);

  InitQueue(&queue2, QUEUE2_SIZE);
  LinkQueue(&queue2, &threebytes, TUPLE2_SIZE);
  LinkQueue(&queue2, &pop_result2, TUPLE2_SIZE);

  // test: push "hello" which is five byte
  fivebytes.bytes[0] = 'h';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 'l';
  fivebytes.bytes[3] = 'l';
  fivebytes.bytes[4] = 'o';
  queue1.Push(&fivebytes);

  // test: push "world" which is five byte
  threebytes.bytes[0] = 'w';
  threebytes.bytes[1] = 'o';
  threebytes.bytes[2] = 'r';
  queue2.Push(&threebytes);

  // test: queue1 pop out all data
  while(queue1.Pop(&pop_result1)) {
    for (i = 0; i < pop_result1.size; ++i) {
      MY_PRINT2("%c", pop_result1.bytes[i]);
    }
    MY_PRINT("\n");
  }

  // test: queue2 pop out all data
  while(queue2.Pop(&pop_result2)) {
    for (i = 0; i < pop_result2.size; ++i) {
     MY_PRINT2("%c", pop_result2.bytes[i]);
    }
    MY_PRINT("\n");
  }
  return 0;
}


//
// Test ForcePush
//
int TestFunction3()
{
  // the queue is a container to store QTuple data
  // you can redefine the QTupleSize to change the max size that a QTuple can store. 
  // here, for example the size is 5.
  XDATA Queue1 queue;
  // we will push two fivebytes to queue
  Tuple1 fivebytes;
  // then pop them out to pop_result 
  Tuple1 pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue, QUEUE1_SIZE);
  LinkQueue(&queue, &fivebytes, TUPLE1_SIZE);
  LinkQueue(&queue, &pop_result, TUPLE1_SIZE);

  // test: push "hello" which is five byte
  fivebytes.bytes[0] = 'h';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 'l';
  fivebytes.bytes[3] = 'l';
  fivebytes.bytes[4] = 'o';
  queue.Push(&fivebytes);

  // test: push "world" which is five byte
  fivebytes.bytes[0] = 'w';
  fivebytes.bytes[1] = 'o';
  fivebytes.bytes[2] = 'r';
  fivebytes.bytes[3] = 'l';
  fivebytes.bytes[4] = 'd';
  queue.Push(&fivebytes);

  // test: push 
  fivebytes.bytes[0] = 't';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 's';
  fivebytes.bytes[3] = 't';
  fivebytes.bytes[4] = '0';
  queue.PushForce(&fivebytes);

  // test: push 
  fivebytes.bytes[0] = 't';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 's';
  fivebytes.bytes[3] = 't';
  fivebytes.bytes[4] = '1';
  queue.PushForce(&fivebytes);

  // test: push 
  fivebytes.bytes[0] = 't';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 's';
  fivebytes.bytes[3] = 't';
  fivebytes.bytes[4] = '2';
  queue.PushForce(&fivebytes);

  // test: push 
  fivebytes.bytes[0] = 't';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 's';
  fivebytes.bytes[3] = 't';
  fivebytes.bytes[4] = '3';
  queue.PushForce(&fivebytes);

  // test: queue1 pop out all data
  while(queue.Pop(&pop_result)) {
    for (i = 0; i < pop_result.size; ++i) {
      MY_PRINT2("%c", pop_result.bytes[i]);
    }
    MY_PRINT("\n");
  }

  return 0;
}

int main()
{
  // The test function shows how to use Queue

  // Test one local queue, tuple size is 5
  MY_PRINT("\nTest1:\n");
  TestFunction1();

  // Test two local queues with two different type tuple
  // tuple size is 5 and 3
  MY_PRINT("\nTest2:\n");
  TestFunction2();

  // Test Force Push data into queue
  // to see the effect, redefine the QUEUE1_SIZE to smaller size, e.g. 4
  MY_PRINT("\nTest3:\n");
  TestFunction3();
  return 0;
}

