#include "stack.h"


//
// Test push and pop
//
int TestFunction1()
{
  // the stack is a container to store STuple data
  // you can redefine the STupleSize to change the max size that a STuple can store. 
  // here, for example the size is 5.
  // Note: the "XDATA" indicate that Stack's data is in external ram, 
  //       which is larger, but slower. If you remove XDATA, then make SContSize smaller.
  // ref: http://pololu.github.io/wixel-sdk/cc2511__types_8h.html
  XDATA Stack stack;
  // we will push two fivebytes to stack
  STuple fivebytes;
  // then pop them out to pop_result 
  STuple pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Stack ***
  //***  we have to link Stack and STuple to use member function.  ***
  //*****************************************************************
  InitStack(&stack);
  LinkStackSTuple(&stack, &fivebytes);
  LinkStackSTuple(&stack, &pop_result);

  // test push "hello" which is five byte
  fivebytes.bytes[0] = 'h';
  fivebytes.bytes[1] = 'e';
  fivebytes.bytes[2] = 'l';
  fivebytes.bytes[3] = 'l';
  fivebytes.bytes[4] = 'o';
  stack.Push(&fivebytes);

  // test push "world" which is five byte
  fivebytes.bytes[0] = 'w';
  fivebytes.bytes[1] = 'o';
  fivebytes.bytes[2] = 'r';
  fivebytes.bytes[3] = 'l';
  fivebytes.bytes[4] = 'd';
  stack.Push(&fivebytes);

  // test pop out all data
  while(stack.Pop(&pop_result)) {
    for (i = 0; i < STupleSize; ++i) {
      MY_PRINT2("%c", pop_result.bytes[i]);
    }
    MY_PRINT("\n");
  }
  return 0;
}


//
// Test multiple Stacks
//
int TestFunction2()
{
  // the stack is a container to store STuple data
  // you can redefine the STupleSize to change the max size that a STuple can store. 
  // here, for example the size is 5.
  XDATA Stack stack1;
  XDATA Stack stack2;
  // we will push two fivebytes to stack
  STuple fivebytes1, fivebytes2;
  // then pop them out to pop_result 
  STuple pop_result1, pop_result2;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Stack ***
  //***  we have to link Stack and STuple to use member function.  ***
  //*****************************************************************
  InitStack(&stack1);
  LinkStackSTuple(&stack1, &fivebytes1);
  LinkStackSTuple(&stack1, &pop_result1);

  InitStack(&stack2);
  LinkStackSTuple(&stack2, &fivebytes2);
  LinkStackSTuple(&stack2, &pop_result2);

  // test: push "hello" which is five byte
  fivebytes1.bytes[0] = 'h';
  fivebytes1.bytes[1] = 'e';
  fivebytes1.bytes[2] = 'l';
  fivebytes1.bytes[3] = 'l';
  fivebytes1.bytes[4] = 'o';
  stack1.Push(&fivebytes1);

  // test: push "world" which is five byte
  fivebytes2.bytes[0] = 'w';
  fivebytes2.bytes[1] = 'o';
  fivebytes2.bytes[2] = 'r';
  fivebytes2.bytes[3] = 'l';
  fivebytes2.bytes[4] = 'd';
  stack2.Push(&fivebytes2);

  // test: stack1 pop out all data
  while(stack1.Pop(&pop_result1)) {
    for (i = 0; i < STupleSize; ++i) {
      MY_PRINT2("%c", pop_result1.bytes[i]);
    }
    MY_PRINT("\n");
  }

  // test: stack2 pop out all data
  while(stack2.Pop(&pop_result2)) {
    for (i = 0; i < STupleSize; ++i) {
      MY_PRINT2("%c", pop_result2.bytes[i]);
    }
    MY_PRINT("\n");
  }
  return 0;
}


int main()
{
  // The test function shows how to use Stack

  // Test one local stacks
  MY_PRINT("Test1:\n");
  TestFunction1();

  // Test two local stacks
  MY_PRINT("Test2:\n");
  TestFunction2();
  return 0;
}


