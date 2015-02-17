#include<stdio.h>
#include<wixel.h>

/*************************************
 * Container: Stack
 *************************************/
#define ContSize 64
#define TupleSize 5 

// forward decl.
struct Stack;

//
// FiveBytes Structure
//
struct Tuple{
  struct Stack* caller; 
  char bytes[TupleSize];
};
typedef struct Tuple Tuple;

// 
// Container: Stack 
//  - contains several fivebytes data
// 
struct Stack
{
  // member data
  Tuple data[ContSize];
  int top_idx; // indicate the available slot in data[]

  // member function declariation
  int (*Push)(Tuple* p);
  int (*Pop)(Tuple* p);
};
typedef struct Stack Stack;

//
// member funciton: Push 
// return 1: success, 
//        0: fail(stack is full)
//
int Push(Tuple* p) {
  int i = 0;
  Stack* stack = (0==p ? 0:  p->caller);
  if (0!=stack && stack->top_idx < ContSize) {
    // copy p to stack
    for (i = 0; i < TupleSize; ++i) {
      stack->data[stack->top_idx].bytes[i] = p->bytes[i];
    }
    stack->top_idx++;
    return 1;
  }
  return 0;
}

//
// member funciton: Pop
// return 1: success, 
//        0: fail(stack is already empty)
//
int Pop(Tuple* p) {
  int i = 0;
  Stack* stack = 0==p ? 0 :  p->caller;
  if (0!=stack && stack->top_idx > 0) {
    stack->top_idx--;
    // copy stack to p
    for (i = 0; i < TupleSize; ++i) {
      p->bytes[i] = stack->data[stack->top_idx].bytes[i];
    }
    return 1;
  }
  return 0;
}

//
// Call this before using Stack
// return 1: success
//        0: fail
//
int InitStack(Stack* stack) {
  if (0!=stack) {
    stack->top_idx = 0;
    stack->Push = Push;
    stack->Pop = Pop;
    return 1;
  }
  return 0;
}

//
// Associate Stack and Tuple. 
// return 1: success
//        0: fail
//
int LinkStackTuple(Stack* stack, Tuple* tuple) {
  if (0!=stack && 0!=tuple) {
    tuple->caller = stack;
    return 1;
  }
  return 0;
}


//
// Test push and pop
//
int TestFunction1()
{
  // the stack is a container to store Tuple data
  // you can redefine the TupleSize to change the max size that a Tuple can store. 
  // here, for example the size is 5.
  // Note: the "XDATA" indicate that Stack's data is in external ram, 
  //       which is larger, but slower. If you remove XDATA, then make ContSize smaller.
  // ref: http://pololu.github.io/wixel-sdk/cc2511__types_8h.html
  XDATA Stack stack;
  // we will push two fivebytes to stack
  Tuple fivebytes;
  // then pop them out to pop_result 
  Tuple pop_result;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Stack ***
  //***  we have to link Stack and Tuple to use member function.  ***
  //*****************************************************************
  InitStack(&stack);
  LinkStackTuple(&stack, &fivebytes);
  LinkStackTuple(&stack, &pop_result);

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
    for (i = 0; i < TupleSize; ++i) {
     // printf("%c", pop_result.bytes[i]);
    }
    // printf("\n");
  }
  return 0;
}


//
// Test multiple Stacks
//
int TestFunction2()
{
  // the stack is a container to store Tuple data
  // you can redefine the TupleSize to change the max size that a Tuple can store. 
  // here, for example the size is 5.
  XDATA Stack stack1;
  XDATA Stack stack2;
  // we will push two fivebytes to stack
  Tuple fivebytes1, fivebytes2;
  // then pop them out to pop_result 
  Tuple pop_result1, pop_result2;

  int i = 0;

  //*****************************************************************
  //*** Remember to call init and link fuction before using Stack ***
  //***  we have to link Stack and Tuple to use member function.  ***
  //*****************************************************************
  InitStack(&stack1);
  LinkStackTuple(&stack1, &fivebytes1);
  LinkStackTuple(&stack1, &pop_result1);

  InitStack(&stack2);
  LinkStackTuple(&stack2, &fivebytes2);
  LinkStackTuple(&stack2, &pop_result2);

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
    for (i = 0; i < TupleSize; ++i) {
      // printf("%c", pop_result1.bytes[i]);
    }
    // printf("\n");
  }

  // test: stack2 pop out all data
  while(stack2.Pop(&pop_result2)) {
    for (i = 0; i < TupleSize; ++i) {
      // printf("%c", pop_result2.bytes[i]);
    }
    // printf("\n");
  }
  return 0;
}


int main()
{
  // The test function shows how to use Stack

  // Test one local stacks
  // printf("Test1:\n");
  TestFunction1();
  // Test two local stacks
  // printf("Test2:\n");
  TestFunction2();
  return 0;
}


