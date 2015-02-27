#include "queue.h"
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
  XDATA Queue queue;
  // we will push two fivebytes to queue
  QTuple fivebytes;
  // then pop them out to pop_result 
  QTuple pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue);
  LinkQueue(&queue, &fivebytes);
  LinkQueue(&queue, &pop_result);

  for (i = 0; i < QContSize; i++) {
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
    for (i = 0; i < QTupleSize; ++i) {
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
  // the queue is a container to store QTuple data
  // you can redefine the QTupleSize to change the max size that a QTuple can store. 
  // here, for example the size is 5.
  XDATA Queue queue1;
  XDATA Queue queue2;
  // we will push two fivebytes to queue
  QTuple fivebytes1, fivebytes2;
  // then pop them out to pop_result 
  QTuple pop_result1, pop_result2;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue1);
  LinkQueue(&queue1, &fivebytes1);
  LinkQueue(&queue1, &pop_result1);

  InitQueue(&queue2);
  LinkQueue(&queue2, &fivebytes2);
  LinkQueue(&queue2, &pop_result2);

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
    for (i = 0; i < QTupleSize; ++i) {
      MY_PRINT2("%c", pop_result1.bytes[i]);
    }
    MY_PRINT("\n");
  }

  // test: queue2 pop out all data
  while(queue2.Pop(&pop_result2)) {
    for (i = 0; i < QTupleSize; ++i) {
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
  XDATA Queue queue1;
  // we will push two fivebytes to queue
  QTuple fivebytes1;
  // then pop them out to pop_result 
  QTuple pop_result1;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Queue ***
  //***  we have to link Queue and QTuple to use member function.  ***
  //*****************************************************************
  InitQueue(&queue1);
  LinkQueue(&queue1, &fivebytes1);
  LinkQueue(&queue1, &pop_result1);

  // test: push "hello" which is five byte
  fivebytes1.bytes[0] = 'h';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 'l';
  fivebytes1.bytes[3] = 'l';
  fivebytes1.bytes[4] = 'o';
  queue1.Push(&fivebytes1);

  // test: push "world" which is five byte
  fivebytes1.bytes[0] = 'w';
  fivebytes1.bytes[1] = 'o';
  fivebytes1.bytes[2] = 'r';
  fivebytes1.bytes[3] = 'l';
  fivebytes1.bytes[4] = 'd';
  queue1.Push(&fivebytes1);

  // test: push 
  fivebytes1.bytes[0] = 't';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 's';
  fivebytes1.bytes[3] = 't';
  fivebytes1.bytes[4] = '0';
  queue1.PushForce(&fivebytes1);

  // test: push 
  fivebytes1.bytes[0] = 't';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 's';
  fivebytes1.bytes[3] = 't';
  fivebytes1.bytes[4] = '1';
  queue1.PushForce(&fivebytes1);

  // test: push 
  fivebytes1.bytes[0] = 't';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 's';
  fivebytes1.bytes[3] = 't';
  fivebytes1.bytes[4] = '2';
  queue1.PushForce(&fivebytes1);

  // test: push 
  fivebytes1.bytes[0] = 't';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 's';
  fivebytes1.bytes[3] = 't';
  fivebytes1.bytes[4] = '3';
  queue1.PushForce(&fivebytes1);


  // test: queue1 pop out all data
  while(queue1.Pop(&pop_result1)) {
    for (i = 0; i < QTupleSize; ++i) {
      MY_PRINT2("%c", pop_result1.bytes[i]);
    }
    MY_PRINT("\n");
  }

  return 0;
}

int main()
{
  // The test function shows how to use Queue

  // Test one local queue
  MY_PRINT("\nTest1:\n");
  TestFunction1();

  // Test two local queues
  MY_PRINT("\nTest2:\n");
  TestFunction2();

  // Test Force Push data into queue
  // to see the effect, redefine the QContSize to smaller size, e.g. 4
  MY_PRINT("\nTest3:\n");
  TestFunction3();
  return 0;
}

