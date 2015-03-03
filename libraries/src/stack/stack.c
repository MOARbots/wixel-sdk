#include "stack.h"

//
// member funciton: SPush 
// return 1: success, 
//        0: fail(stack is full)
//
int SPush(STuple* p) {
  int i = 0;
  Stack* stack = (0==p ? 0:  p->caller);
  if (0!=stack && stack->top_idx < SContSize) {
    // copy p to stack
    for (i = 0; i < STupleSize; ++i) {
      stack->Sdata[stack->top_idx].bytes[i] = p->bytes[i];
    }
    stack->top_idx++;
    return 1;
  }
  return 0;
}

//
// member funciton: SPop
// return 1: success, 
//        0: fail(stack is already empty)
//
int SPop(STuple* p) {
  int i = 0;
  Stack* stack = 0==p ? 0 :  p->caller;
  if (0!=stack && stack->top_idx > 0) {
    stack->top_idx--;
    // copy stack to p
    for (i = 0; i < STupleSize; ++i) {
      p->bytes[i] = stack->Sdata[stack->top_idx].bytes[i];
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
    stack->Push = SPush;
    stack->Pop = SPop;
    return 1;
  }
  return 0;
}

//
// Associate Stack and STuple. 
// return 1: success
//        0: fail
//
int LinkStackSTuple(Stack* stack, STuple* tuple) {
  if (0!=stack && 0!=tuple) {
    tuple->caller = stack;
    return 1;
  }
  return 0;
}


